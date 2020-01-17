/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2018 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "DistrhoPlugin.hpp"
#include <roc/address.h>
#include <roc/context.h>
#include <roc/log.h>
#include <roc/sender.h>
#include <roc/receiver.h>


#define EXAMPLE_SENDER_IP "0.0.0.0"

#define METER_COLOR_GREEN 0
#define METER_COLOR_BLUE  1
#define EXAMPLE_SENDER_PORT 0

/* Receiver parameters. */
#define SERVER_RECEIVER_IP "0.0.0.0"
#define EXAMPLE_CLIENT_SERVER_SOURCE_PORT 10000
#define EXAMPLE_CLIENT_SERVER_REPAIR_PORT 10001


#define EXAMPLE_NUM_CHANNELS 2

/* Signal parameters */
#define EXAMPLE_SAMPLE_RATE 44100
#define EXAMPLE_SINE_RATE 440
#define EXAMPLE_SINE_SAMPLES (EXAMPLE_SAMPLE_RATE * 5)
#define EXAMPLE_BUFFER_SIZE 100

#define EXAMPLE_RECEIVER_IP "0.0.0.0"
#define EXAMPLE_RECEIVER_SOURCE_PORT 20000
#define EXAMPLE_RECEIVER_REPAIR_PORT 20001

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

/**
  Plugin to demonstrate parameter outputs using meters.
 */
class RemoteEntanglementPlugin : public Plugin {
public:

    RemoteEntanglementPlugin()
    : Plugin(3, 0, 0), // 3 parameters, 0 programs, 0 states
    fColor(0.0f),
    fOutLeft(0.0f),
    fOutRight(0.0f),
    fNeedsReset(true) {
    }

protected:
    /* --------------------------------------------------------------------------------------------------------
     * Information */

    /**
       Get the plugin label.
       A plugin label follows the same rules as Parameter::symbol, with the exception that it can start with numbers.
     */
    const char* getLabel() const override {
        return "meters";
    }

    /**
       Get an extensive comment/description about the plugin.
     */
    const char* getDescription() const override {
        return "Plugin to demonstrate parameter outputs using meters.";
    }

    /**
       Get the plugin author/maker.
     */
    const char* getMaker() const override {
        return "DISTRHO";
    }

    /**
       Get the plugin homepage.
     */
    const char* getHomePage() const override {
        return "https://github.com/DISTRHO/DPF";
    }

    /**
       Get the plugin license name (a single line of text).
       For commercial plugins this should return some short copyright information.
     */
    const char* getLicense() const override {
        return "ISC";
    }

    /**
       Get the plugin version, in hexadecimal.
     */
    uint32_t getVersion() const override {
        return d_version(1, 0, 0);
    }

    /**
       Get the plugin unique Id.
       This value is used by LADSPA, DSSI and VST plugin formats.
     */
    int64_t getUniqueId() const override {
        return d_cconst('d', 'M', 't', 'r');
    }

    /* --------------------------------------------------------------------------------------------------------
     * Init */

    /**
       Initialize the parameter @a index.
       This function will be called once, shortly after the plugin is created.
     */

    void initSender() {
        roc_log_set_level(ROC_LOG_DEBUG);

        /* Initialize context config.
         * Initialize to zero to use default values for all fields. */
        roc_context_config context_config;
        memset(&context_config, 0, sizeof (context_config));
        //        context_config.max_packet_size=8;
        //        context_config.max_frame_size=8;

        //        context_config.max_frame_size = 4;
        /* Create context.
         * Context contains memory pools and the network worker thread(s).
         * We need a context to create a sender. */
        roc_context* context = roc_context_open(&context_config);
        if (!context) {
            log("roc_context_open");
        }

        /* Initialize sender config.
         * Initialize to zero to use default values for unset fields. */
        roc_sender_config sender_config;
        memset(&sender_config, 0, sizeof (sender_config));

        /* Setup input frame format. */
        log("buffer size init:");
        logInt(getBufferSize());

        log("buffer size init:");
        logInt(getSampleRate());

        sender_config.frame_sample_rate = getSampleRate();
        sender_config.frame_channels = ROC_CHANNEL_SET_STEREO;
        sender_config.frame_encoding = ROC_FRAME_ENCODING_PCM_FLOAT;
        sender_config.packet_sample_rate = getSampleRate();

        /* Turn on sender timing.
         * Sender must send packets with steady rate, so we should either implement
         * clocking or ask the library to do so. We choose the second here. */
        sender_config.automatic_timing = 1;

        /* Create sender. */
        sender_ = roc_sender_open(context, &sender_config);
        if (!sender_) {
            log("roc_sender_open");
        }

        /* Bind sender to a random port. */
        roc_address sender_addr;
        if (roc_address_init(&sender_addr, ROC_AF_AUTO, EXAMPLE_SENDER_IP,
                EXAMPLE_SENDER_PORT)
                != 0) {
            log("roc_address_init");
        }
        if (roc_sender_bind(sender_, &sender_addr) != 0) {
            log("roc_sender_bind");
        }

        /* Connect sender to the receiver source (audio) packets port.
         * The receiver should expect packets with RTP header and Reed-Solomon (m=8) FECFRAME
         * Source Payload ID on that port. */
        roc_address recv_source_addr;
        if (roc_address_init(&recv_source_addr, ROC_AF_AUTO, SERVER_RECEIVER_IP,
                EXAMPLE_CLIENT_SERVER_SOURCE_PORT)
                != 0) {
            log("roc_address_init");
        }
        if (roc_sender_connect(sender_, ROC_PORT_AUDIO_SOURCE, ROC_PROTO_RTP_RS8M_SOURCE,
                &recv_source_addr)
                != 0) {
            log("roc_sender_connect");
        }

        /* Connect sender to the receiver repair (FEC) packets port.
         * The receiver should expect packets with Reed-Solomon (m=8) FECFRAME
         * Repair Payload ID on that port. */
        roc_address recv_repair_addr;
        if (roc_address_init(&recv_repair_addr, ROC_AF_AUTO, SERVER_RECEIVER_IP,
                EXAMPLE_CLIENT_SERVER_REPAIR_PORT)
                != 0) {
            log("roc_address_init");
        }
        if (roc_sender_connect(sender_, ROC_PORT_AUDIO_REPAIR, ROC_PROTO_RS8M_REPAIR,
                &recv_repair_addr)
                != 0) {
            log("roc_sender_connect");
        }

    }

    void initReceiver() {
        /* Initialize SoX. */

        /* Enable debug logging. */
        roc_log_set_level(ROC_LOG_DEBUG);

        /* Initialize context config.
         * Initialize to zero to use default values for all fields. */
        roc_context_config context_config;
        memset(&context_config, 0, sizeof (context_config));

        /* Create context.
         * Context contains memory pools and the network worker thread(s).
         * We need a context to create a receiver. */
        roc_context* context = roc_context_open(&context_config);
        if (!context) {
            log("roc_context_open");
        }

        /* Initialize receiver config.
         * We use default values. */
        roc_receiver_config receiver_config;
        memset(&receiver_config, 0, sizeof (receiver_config));

        /* Setup output frame format. */
        receiver_config.frame_sample_rate = EXAMPLE_SAMPLE_RATE;
        receiver_config.frame_channels = ROC_CHANNEL_SET_STEREO;
        receiver_config.frame_encoding = ROC_FRAME_ENCODING_PCM_FLOAT;

        /* Create receiver. */
        receiver_ = roc_receiver_open(context, &receiver_config);
        if (!receiver_) {
            log("roc_receiver_open");
        }

        /* Bind receiver to the source (audio) packets port.
         * The receiver will expect packets with RTP header and Reed-Solomon (m=8) FECFRAME
         * Source Payload ID on this port. */
        roc_address recv_source_addr;
        if (roc_address_init(&recv_source_addr, ROC_AF_AUTO, EXAMPLE_RECEIVER_IP,
                EXAMPLE_RECEIVER_SOURCE_PORT)
                != 0) {
            log("roc_address_init");
        }
        if (roc_receiver_bind(receiver_, ROC_PORT_AUDIO_SOURCE, ROC_PROTO_RTP_RS8M_SOURCE,
                &recv_source_addr)
                != 0) {
            log("roc_receiver_bind");
        }

        /* Bind receiver to the repair (FEC) packets port.
         * The receiver will expect packets with Reed-Solomon (m=8) FECFRAME
         * Repair Payload ID on this port. */
        roc_address recv_repair_addr;
        if (roc_address_init(&recv_repair_addr, ROC_AF_AUTO, EXAMPLE_RECEIVER_IP,
                EXAMPLE_RECEIVER_REPAIR_PORT)
                != 0) {
            log("roc_address_init");
        }
        if (roc_receiver_bind(receiver_, ROC_PORT_AUDIO_REPAIR, ROC_PROTO_RS8M_REPAIR,
                &recv_repair_addr)
                != 0) {
            log("roc_receiver_bind");
        }

        /* Initialize SoX parameters. */
        //        
        //
        //        /* Destroy receiver. */
        //        if (roc_receiver_close(receiver_) != 0) {
        //            log("roc_receiver_close");
        //        }
        //
        //        /* Destroy context. */
        //        if (roc_context_close(context) != 0) {
        //            log("roc_context_close");
        //        }

    }

    void activate() override {
        if (!sender_) {
            initSender();
        }
        if (!receiver_) {
            initReceiver();
        }

    }

    void initParameter(uint32_t index, Parameter& parameter) override {

        /**
           All parameters in this plugin have the same ranges.
         */
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        parameter.ranges.def = 0.0f;

        /**
           Set parameter data.
         */
        switch (index) {
            case 0:
                parameter.hints = kParameterIsAutomable | kParameterIsInteger;
                parameter.name = "color";
                parameter.symbol = "color";
                parameter.enumValues.count = 2;
                parameter.enumValues.restrictedMode = true;
            {
                ParameterEnumerationValue * const values = new ParameterEnumerationValue[2];
                parameter.enumValues.values = values;

                values[0].label = "Green";
                values[0].value = METER_COLOR_GREEN;
                values[1].label = "Blue";
                values[1].value = METER_COLOR_BLUE;
            }
                break;
            case 1:
                parameter.hints = kParameterIsAutomable | kParameterIsOutput;
                parameter.name = "out-left";
                parameter.symbol = "out_left";
                break;
            case 2:
                parameter.hints = kParameterIsAutomable | kParameterIsOutput;
                parameter.name = "out-right";
                parameter.symbol = "out_right";
                break;
        }
    }

    void log(const char* msg) {
        fprintf(stderr, "log: %s\n", msg);
    }

    void logInt(uint32_t msg) {
        fprintf(stderr, "log: %i\n", msg);
    }

    void logFloat(float msg) {
        fprintf(stderr, "log: %f\n", msg);
    }

    /**
       Set a state key and default value.
       This function will be called once, shortly after the plugin is created.
     */
    void initState(uint32_t, String&, String&) override {
        // we are using states but don't want them saved in the host
    }

    /* --------------------------------------------------------------------------------------------------------
     * Internal data */

    /**
       Get the current value of a parameter.
     */
    float getParameterValue(uint32_t index) const override {
        switch (index) {
            case 0: return fColor;
            case 1: return fOutLeft;
            case 2: return fOutRight;
        }

        return 0.0f;
    }

    /**
       Change a parameter value.
     */
    void setParameterValue(uint32_t index, float value) override {
        // this is only called for input paramters, and we only have one of those.
        if (index != 0) return;

        fColor = value;
    }

    /**
       Change an internal state.
     */
    void setState(const char* key, const char*) override {
        if (std::strcmp(key, "reset") != 0)
            return;

        fNeedsReset = true;
    }

    /* --------------------------------------------------------------------------------------------------------
     * Process */

    /**
       Run/process function for plugins without MIDI input.
     */
    void run(const float** inputs, float** outputs, uint32_t frames) override {
        if (sender_) {
            float tmp;
            float tmpLeft = 0.0f;
            float tmpRight = 0.0f;

            for (uint32_t i = 0; i < frames; ++i) {
                float samples[2];
                samples[0] = inputs[0][i];
                samples[1] = inputs[1][i];
                roc_frame frame;
                memset(&frame, 0, sizeof (frame));
                frame.samples = samples;
                frame.samples_size = sizeof (samples);
                if (roc_sender_write(sender_, &frame) != 0) {
                    log("roc_sender_write");
                }

                // left
                tmp = std::abs(inputs[0][i]);

                if (tmp > tmpLeft)
                    tmpLeft = tmp;

                // right
                tmp = std::abs(inputs[1][i]);

                if (tmp > tmpRight)
                    tmpRight = tmp;

                roc_frame out_frame;
                memset(&out_frame, 0, sizeof (out_frame));
                if (roc_receiver_read(receiver_, &out_frame) != 0) {
                    break;
                } else {
                    std::memcpy(outputs[0], &out_frame.samples, sizeof (float) * sizeof (&out_frame.samples));
                }
            }

            if (tmpLeft > 1.0f)
                tmpLeft = 1.0f;
            if (tmpRight > 1.0f)
                tmpRight = 1.0f;

            if (fNeedsReset) {
                fOutLeft = tmpLeft;
                fOutRight = tmpRight;
                fNeedsReset = false;
            } else {
                if (tmpLeft > fOutLeft)
                    fOutLeft = tmpLeft;
                if (tmpRight > fOutRight)
                    fOutRight = tmpRight;
            }
        }

    }

    roc_context * context_{};
    roc_sender * sender_{};
    roc_receiver * receiver_{};

    // -------------------------------------------------------------------------------------------------------

private:
    /**
       Parameters.
     */
    float fColor, fOutLeft, fOutRight;


    /**
       Boolean used to reset meter values.
       The UI will send a "reset" message which sets this as true.
     */
    volatile bool fNeedsReset;

    /**
       Set our plugin class as non-copyable and add a leak detector just in case.
     */
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RemoteEntanglementPlugin)
};

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

Plugin* createPlugin() {
    return new RemoteEntanglementPlugin();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
