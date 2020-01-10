/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RemoteEntanglement.h
 * Author: my5t3ry
 *
 * Created on 10. Januar 2020, 05:09
 */

#include <math.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <roc/address.h>
#include <roc/context.h>
#include <roc/log.h>
#include <roc/sender.h>



#include <algorithm>
#include <future>
#include <vector>

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO
/* Sender parameters. */
#define EXAMPLE_SENDER_IP "127.0.0.1"
#define EXAMPLE_SENDER_PORT 0

/* Receiver parameters. */
#define EXAMPLE_RECEIVER_IP "0.0.0.0"
#define EXAMPLE_RECEIVER_SOURCE_PORT 10001
#define EXAMPLE_RECEIVER_REPAIR_PORT 10002

/* Signal parameters */
#define EXAMPLE_SAMPLE_RATE 44100
#define EXAMPLE_SINE_RATE 440
#define EXAMPLE_SINE_SAMPLES (EXAMPLE_SAMPLE_RATE * 5)
#define EXAMPLE_BUFFER_SIZE 100



// -----------------------------------------------------------------------


class RemoteEntanglementPlugin : public Plugin {
public:

    RemoteEntanglementPlugin()
    : Plugin(1, 0, 0) {

    }
protected:

    /* ----------------------------------------------------------------------------------------
     * Information */
    const char* getLabel() const override {
        return "Gain";
    }

    const char* getMaker() const override {
        return "DPF";
    }

    const char* getLicense() const override {
        return "MIT";
    }

    uint32_t getVersion() const override {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const override {
        return d_cconst('G', 'a', 'i', 'n');
    }

    /* ----------------------------------------------------------------------------------------
     * Init */
    void log(const char* msg) {                                                                                 \
        fprintf(stderr, "log: %s\n", msg);                                              \
                        }

    void logInt(uint32_t msg) {                                                                                 \
        fprintf(stderr, "log: %i\n", msg);                                              \
                            }

    void logFloat(float msg) {                                                                                 \
        fprintf(stderr, "log: %f\n", msg);                                              \
                            }

    void activate() {
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
        if (roc_address_init(&recv_source_addr, ROC_AF_AUTO, EXAMPLE_RECEIVER_IP,
                EXAMPLE_RECEIVER_SOURCE_PORT)
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
        if (roc_address_init(&recv_repair_addr, ROC_AF_AUTO, EXAMPLE_RECEIVER_IP,
                EXAMPLE_RECEIVER_REPAIR_PORT)
                != 0) {
            log("roc_address_init");
        }
        if (roc_sender_connect(sender_, ROC_PORT_AUDIO_REPAIR, ROC_PROTO_RS8M_REPAIR,
                &recv_repair_addr)
                != 0) {
            log("roc_sender_connect");
        }

    }

    /**
       Initialize a parameter.
       This function will be called once, shortly after the plugin is created.
     */
    void initParameter(uint32_t index, Parameter& parameter) override {


    }
    /* ----------------------------------------------------------------------------------------
     * Internal data */

    float getParameterValue(uint32_t index) const override {
        // same as before, ignore index check
        return fGain;
    }

    void setParameterValue(uint32_t index, float value) override {
        // same as before, ignore index check
        fGain = value;
    }


    void run(const float** inputs, float** outputs, uint32_t frames) override {
        if (sender_) {
            int32_t index = 0 ;
            for (uint32_t i = 0; i < frames; i++) {
                float samples[2];
                samples[0] = inputs[0][i];
                samples[1] = inputs[1][i];
                roc_frame frame;
                memset(&frame, 0, sizeof (frame));
                frame.samples = samples;
                frame.samples_size = sizeof(samples);
                if (roc_sender_write(sender_, &frame) != 0) {
                    log("roc_sender_write");
                }
            }
        }
    }

    roc_context * context_{};
    roc_sender * sender_{};


private:
    float fGain;

};

Plugin* createPlugin() {
    return new RemoteEntanglementPlugin();
}


// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

