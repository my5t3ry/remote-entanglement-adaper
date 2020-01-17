/*
 * Copyright (c) 2018 Roc authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* Roc receiver example.
 *
 * This example receives an audio stream and plays it using SoX.
 * Receiver address and ports and other parameters are hardcoded.
 *
 * Building:
 *   gcc receiver_sox.c -lroc -lsox
 *
 * Running:
 *   ./a.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sox.h>

#include <roc/address.h>
#include <roc/context.h>
#include <roc/log.h>
#include <roc/receiver.h>
#include <roc/sender.h>


/* Receiver parameters. */
#define EXAMPLE_RECEIVER_IP "0.0.0.0"
#define EXAMPLE_RECEIVER_SOURCE_PORT 10001
#define EXAMPLE_RECEIVER_REPAIR_PORT 10002
#define EXAMPLE_SENDER_IP "0.0.0.0"
#define EXAMPLE_SENDER_PORT 0

/* Player parameters. */
#define EXAMPLE_OUTPUT_DEVICE "default"
#define EXAMPLE_OUTPUT_TYPE "alsa"
#define EXAMPLE_SAMPLE_RATE 44100
#define EXAMPLE_NUM_CHANNELS 2
#define EXAMPLE_BUFFER_SIZE 1000

#define EXAMPLE_CLIENT_RECEIVER_IP "127.0.0.1"
#define EXAMPLE_CLIENT_RECEIVER_SOURCE_PORT 10003
#define EXAMPLE_CLIENT_RECEIVER_REPAIR_PORT 10004

/* Signal parameters */
#define EXAMPLE_SAMPLE_RATE 44100
#define EXAMPLE_SINE_RATE 440
#define EXAMPLE_SINE_SAMPLES (EXAMPLE_SAMPLE_RATE * 5)
#define EXAMPLE_BUFFER_SIZE 100



#define oops(msg)                                                                        \
    do {                                                                                 \
        fprintf(stderr, "oops: %s\n", msg);                                              \
        exit(1);                                                                         \
    } while (0)

int main() {
    
     roc_context_config sender_context_config;
    memset(&sender_context_config, 0, sizeof(sender_context_config));

    /* Create sender_context.
     * Context contains memory pools and the network worker thread(s).
     * We need a sender_context to create a sender. */
    roc_context* sender_context = roc_context_open(&sender_context_config);
    if (!sender_context) {
        oops("roc_sendercontext_open");
    }

    /* Initialize sender config.
     * Initialize to zero to use default values for unset fields. */
    roc_sender_config sender_config;
    memset(&sender_config, 0, sizeof(sender_config));

    /* Setup input frame format. */
    sender_config.frame_sample_rate = EXAMPLE_SAMPLE_RATE;
    sender_config.frame_channels = ROC_CHANNEL_SET_STEREO;
    sender_config.frame_encoding = ROC_FRAME_ENCODING_PCM_FLOAT;

    /* Turn on sender timing.
     * Sender must send packets with steady rate, so we should either implement
     * clocking or ask the library to do so. We choose the second here. */
    sender_config.automatic_timing = 1;

    /* Create sender. */
    roc_sender* sender = roc_sender_open(sender_context, &sender_config);
    if (!sender) {
        oops("roc_sender_open");
    }

    /* Bind sender to a random port. */
    roc_address sender_addr;
    if (roc_address_init(&sender_addr, ROC_AF_AUTO, EXAMPLE_SENDER_IP,
                         EXAMPLE_SENDER_PORT)
        != 0) {
        oops("roc_senderaddress_init");
    }
    if (roc_sender_bind(sender, &sender_addr) != 0) {
        oops("roc_sendersender_bind");
    }

    /* Connect sender to the receiver source (audio) packets port.
     * The receiver should expect packets with RTP header and Reed-Solomon (m=8) FECFRAME
     * Source Payload ID on that port. */
    roc_address client_recv_source_addr;
    if (roc_address_init(&client_recv_source_addr, ROC_AF_AUTO, EXAMPLE_CLIENT_RECEIVER_IP,
                         EXAMPLE_CLIENT_RECEIVER_SOURCE_PORT)
        != 0) {
        oops("roc_address_init");
    }
    if (roc_sender_connect(sender, ROC_PORT_AUDIO_SOURCE, ROC_PROTO_RTP_RS8M_SOURCE,
                           &client_recv_source_addr)
        != 0) {
        oops("roc_sender_connect");
    }

    /* Connect sender to the receiver repair (FEC) packets port.
     * The receiver should expect packets with Reed-Solomon (m=8) FECFRAME
     * Repair Payload ID on that port. */
    roc_address client_recv_repair_addr;
    if (roc_address_init(&client_recv_repair_addr, ROC_AF_AUTO, EXAMPLE_CLIENT_RECEIVER_IP,
                         EXAMPLE_CLIENT_RECEIVER_REPAIR_PORT)
        != 0) {
        oops("roc_address_init");
    }
    if (roc_sender_connect(sender, ROC_PORT_AUDIO_REPAIR, ROC_PROTO_RS8M_REPAIR,
                           &client_recv_repair_addr)
        != 0) {
        oops("roc_sender_connect");
    }


    /* Enable debug logging. */
    roc_log_set_level(ROC_LOG_DEBUG);

    /* Initialize context config.
     * Initialize to zero to use default values for all fields. */
    roc_context_config context_config;
    memset(&context_config, 0, sizeof(context_config));

    /* Create context.
     * Context contains memory pools and the network worker thread(s).
     * We need a context to create a receiver. */
    roc_context* context = roc_context_open(&context_config);
    if (!context) {
        oops("roc_context_open");
    }

    /* Initialize receiver config.
     * We use default values. */
    roc_receiver_config receiver_config;
    memset(&receiver_config, 0, sizeof(receiver_config));

    /* Setup output frame format. */
    receiver_config.frame_sample_rate = EXAMPLE_SAMPLE_RATE;
    receiver_config.frame_channels = ROC_CHANNEL_SET_STEREO;
    receiver_config.frame_encoding = ROC_FRAME_ENCODING_PCM_FLOAT;

    /* Create receiver. */
    roc_receiver* receiver = roc_receiver_open(context, &receiver_config);
    if (!receiver) {
        oops("roc_receiver_open");
    }

    /* Bind receiver to the source (audio) packets port.
     * The receiver will expect packets with RTP header and Reed-Solomon (m=8) FECFRAME
     * Source Payload ID on this port. */
    roc_address recv_source_addr;
    if (roc_address_init(&recv_source_addr, ROC_AF_AUTO, EXAMPLE_RECEIVER_IP,
                         EXAMPLE_RECEIVER_SOURCE_PORT)
        != 0) {
        oops("roc_address_init");
    }
    if (roc_receiver_bind(receiver, ROC_PORT_AUDIO_SOURCE, ROC_PROTO_RTP_RS8M_SOURCE,
                          &recv_source_addr)
        != 0) {
        oops("roc_receiver_bind");
    }

    /* Bind receiver to the repair (FEC) packets port.
     * The receiver will expect packets with Reed-Solomon (m=8) FECFRAME
     * Repair Payload ID on this port. */
    roc_address recv_repair_addr;
    if (roc_address_init(&recv_repair_addr, ROC_AF_AUTO, EXAMPLE_RECEIVER_IP,
                         EXAMPLE_RECEIVER_REPAIR_PORT)
        != 0) {
        oops("roc_address_init");
    }
    if (roc_receiver_bind(receiver, ROC_PORT_AUDIO_REPAIR, ROC_PROTO_RS8M_REPAIR,
                          &recv_repair_addr)
        != 0) {
        oops("roc_receiver_bind");
    }


    /* Receive and play samples. */
    for (;;) {
        /* Read samples from receiver.
         * If not enough samples are received, receiver will pad buffer with zeros. */
        float recv_samples[EXAMPLE_BUFFER_SIZE];

        roc_frame frame;
        memset(&frame, 0, sizeof(frame));

        frame.samples = recv_samples;
        frame.samples_size = EXAMPLE_BUFFER_SIZE * sizeof(float);

        if (roc_receiver_read(receiver, &frame) != 0) {
            break;
        } else {
            roc_sender_write(sender, &frame);
        }

    }

    /* Destroy receiver. */
    if (roc_receiver_close(receiver) != 0) {
        oops("roc_receiver_close");
    }

    /* Destroy context. */
    if (roc_context_close(context) != 0) {
        oops("roc_context_close");
    }

    return 0;
}
