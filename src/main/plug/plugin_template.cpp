/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins-plugin-template
 * Created on: 25 нояб. 2020 г.
 *
 * lsp-plugins-plugin-template is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins-plugin-template is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins-plugin-template. If not, see <https://www.gnu.org/licenses/>.
 */

#include <lsp-plug.in/common/alloc.h>
#include <lsp-plug.in/common/debug.h>
#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/dsp-units/units.h>
#include <lsp-plug.in/plug-fw/meta/func.h>

#include <private/plugins/plugin_template.h>

/* The size of temporary buffer for audio processing */
#define BUFFER_SIZE         0x1000U

#define TRACE_PORT(p)       lsp_trace("  port id=%s", (p)->metadata()->id);

namespace lsp
{
    namespace plugins
    {
        //---------------------------------------------------------------------
        // Plugin factory
        static const meta::plugin_t *plugins[] =
        {
            &meta::plugin_template_mono,
            &meta::plugin_template_stereo
        };

        static plug::Module *plugin_factory(const meta::plugin_t *meta)
        {
            return new plugin_template(meta);
        }

        static plug::Factory factory(plugin_factory, plugins, 2);

        //---------------------------------------------------------------------
        // Implementation
        plugin_template::plugin_template(const meta::plugin_t *meta):
            Module(meta)
        {
            // Compute the number of audio channels by the number of inputs
            nChannels       = 0;
            for (const meta::port_t *p = meta->ports; p->id != NULL; ++p)
                if (meta::is_audio_in_port(p))
                    ++nChannels;

            // Initialize other parameters
            vChannels       = NULL;
            vBuffer         = NULL;

            pBypass         = NULL;
            pGainOut        = NULL;

            pData           = NULL;
        }

        plugin_template::~plugin_template()
        {
            destroy();
        }

        void plugin_template::init(plug::IWrapper *wrapper, plug::IPort **ports)
        {
            // Call parent class for initialization
            Module::init(wrapper, ports);

            // Estimate the number of bytes to allocate
            size_t szof_channels    = align_size(sizeof(channel_t) * nChannels, OPTIMAL_ALIGN);
            size_t buf_sz           = BUFFER_SIZE * sizeof(float);
            size_t alloc            = szof_channels + buf_sz;

            // Allocate memory-aligned data
            uint8_t *ptr            = alloc_aligned<uint8_t>(pData, alloc, OPTIMAL_ALIGN);
            if (ptr == NULL)
                return;

            // Initialize pointers to channels and temporary buffer
            vChannels               = reinterpret_cast<channel_t *>(ptr);
            ptr                    += szof_channels;
            vBuffer                 = reinterpret_cast<float *>(ptr);
            ptr                    += buf_sz;

            for (size_t i=0; i < nChannels; ++i)
            {
                channel_t *c            = &vChannels[i];

                // Construct in-place DSP processors
                c->sLine.construct();
                c->sBypass.construct();

                // Initialize fields
                c->nDelay               = 0;
                c->fDryGain             = 0.0f;
                c->fWetGain             = 0.0f;

                c->pIn                  = NULL;
                c->pOut                 = NULL;
                c->pDelay               = NULL;
                c->pDry                 = NULL;
                c->pWet                 = NULL;

                c->pOutDelay            = NULL;
            }

            // Bind ports
            lsp_trace("Binding ports");
            size_t port_id      = 0;

            // Bind input audio ports
            for (size_t i=0; i<nChannels; ++i)
            {
                TRACE_PORT(ports[port_id]);
                vChannels[i].pIn    = ports[port_id++];
            }

            // Bind output audio ports
            for (size_t i=0; i<nChannels; ++i)
            {
                TRACE_PORT(ports[port_id]);
                vChannels[i].pOut   = ports[port_id++];
            }

            // Bind bypass
            TRACE_PORT(ports[port_id]);
            pBypass              = ports[port_id++];

            // Bind ports for audio processing channels
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c            = &vChannels[i];

                if (i > 0)
                {
                    channel_t *pc           = &vChannels[0];

                    // Share some controls across all channels
                    c->pDelay               = pc->pDelay;
                    c->pDry                 = pc->pDry;
                    c->pWet                 = pc->pWet;
                }
                else
                {
                    // Initialize input controls for the first channel
                    TRACE_PORT(ports[port_id]);
                    c->pDelay               = ports[port_id++];
                    TRACE_PORT(ports[port_id]);
                    c->pDry                 = ports[port_id++];
                    TRACE_PORT(ports[port_id]);
                    c->pWet                 = ports[port_id++];
                }
            }

            // Bind output gain
            TRACE_PORT(ports[port_id]);
            pGainOut            = ports[port_id++];

            // Bind output meters
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c            = &vChannels[i];

                if (i > 0)
                {
                    channel_t *pc           = &vChannels[0];
                    // Share some meters across all channels
                    c->pOutDelay            = pc->pOutDelay;
                }
                else
                {
                    TRACE_PORT(ports[port_id]);
                    c->pOutDelay            = ports[port_id++];
                }

                TRACE_PORT(ports[port_id]);
                c->pInLevel             = ports[port_id++];
                TRACE_PORT(ports[port_id]);
                c->pOutLevel            = ports[port_id++];
            }
        }

        void plugin_template::destroy()
        {
            Module::destroy();

            // Destroy channels
            if (vChannels != NULL)
            {
                for (size_t i=0; i<nChannels; ++i)
                {
                    channel_t *c    = &vChannels[i];
                    c->sLine.destroy();
                }
                vChannels   = NULL;
            }

            vBuffer     = NULL;

            // Free previously allocated data chunk
            if (pData != NULL)
            {
                free_aligned(pData);
                pData       = NULL;
            }
        }

        void plugin_template::update_sample_rate(long sr)
        {
            // Update sample rate for the bypass processors
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c    = &vChannels[i];
                c->sLine.init(dspu::millis_to_samples(sr, meta::plugin_template::DELAY_OUT_MAX_TIME));
                c->sBypass.init(sr);
            }
        }

        void plugin_template::update_settings()
        {
            float out_gain          = pGainOut->value();
            bool bypass             = pBypass->value() >= 0.5f;

            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c            = &vChannels[i];

                // Store the parameters for each processor
                c->fDryGain             = c->pDry->value() * out_gain;
                c->fWetGain             = c->pWet->value() * out_gain;
                c->nDelay               = c->pDelay->value();

                // Update processors
                c->sLine.set_delay(c->nDelay);
                c->sBypass.set_bypass(bypass);
            }
        }

        void plugin_template::process(size_t samples)
        {
            // Process each channel independently
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c            = &vChannels[i];

                // Get input and output buffers
                const float *in         = c->pIn->buffer<float>();
                float *out              = c->pOut->buffer<float>();
                if ((in == NULL) || (out == NULL))
                    continue;

                // Input and output gain meters
                float in_gain           = 0.0f;
                float out_gain          = 0.0f;

                // Process the channel with BUFFER_SIZE chunks
                // Note: since input buffer pointer can be the same to output buffer pointer,
                // we need to store the processed signal data to temporary buffer before
                // it gets processed by the dspu::Bypass processor.
                for (size_t n=0; n<samples; )
                {
                    size_t count            = lsp_min(samples - n, BUFFER_SIZE);

                    // Pre-process signal (fill buffer)
                    c->sLine.process_ramping(vBuffer, in, c->fWetGain, c->nDelay, samples);

                    // Apply 'dry' control
                    if (c->fDryGain > 0.0f)
                        dsp::fmadd_k3(vBuffer, in, c->fDryGain, count);

                    // Compute the gain of input and output signal.
                    in_gain             = lsp_max(in_gain, dsp::abs_max(in, samples));
                    out_gain            = lsp_max(out_gain, dsp::abs_max(vBuffer, samples));

                    // Process the
                    //  - dry (unprocessed) signal stored in 'in'
                    //  - wet (processed) signal stored in 'vBuffer'
                    // Output the result to 'out' buffer
                    c->sBypass.process(out, in, vBuffer, count);

                    // Increment pointers
                    in          +=  count;
                    out         +=  count;
                    n           +=  count;
                }

                // Update meters
                c->pInLevel->set_value(in_gain);
                c->pOutLevel->set_value(out_gain);

                // Output the delay value in milliseconds
                c->pOutDelay->set_value(dspu::samples_to_millis(fSampleRate, c->nDelay));
            }
        }

        void plugin_template::dump(dspu::IStateDumper *v) const
        {
            // It is very useful to dump plugin state for debug purposes
            v->write("nChannels", nChannels);
            v->begin_array("vChannels", vChannels, nChannels);
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c            = &vChannels[i];

                v->begin_object(c, sizeof(channel_t));
                {
                    v->write_object("sLine", &c->sLine);
                    v->write_object("sBypass", &c->sBypass);

                    v->write("nDelay", c->nDelay);
                    v->write("fDryGain", c->fDryGain);
                    v->write("fWetWain", c->fWetGain);

                    v->write("pIn", c->pIn);
                    v->write("pOut", c->pOut);
                    v->write("pDelay", c->pDelay);
                    v->write("pDry", c->pDry);
                    v->write("pWet", c->pWet);

                    v->write("pOutDelay", c->pOutDelay);
                    v->write("pInLevel", c->pInLevel);
                    v->write("pOutLevel", c->pOutLevel);
                }
                v->end_object();
            }
            v->end_array();

            v->write("vBuffer", vBuffer);

            v->write("pBypass", pBypass);
            v->write("pGainOut", pGainOut);

            v->write("pData", pData);
        }

    }
}


