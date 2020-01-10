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



#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------


class RemoteEntanglementPlugin : public Plugin {
public:

    RemoteEntanglementPlugin()
    : Plugin(1, 0, 0), // 1 parameter, 0 programs and 0 states
    fGain(1.0f) {
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

    /**
       Initialize a parameter.
       This function will be called once, shortly after the plugin is created.
     */
    void initParameter(uint32_t index, Parameter& parameter) override {
        // we only have one parameter so we can skip checking the index
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Gain";
        parameter.symbol = "gain";
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 2.0f;
        parameter.ranges.def = 1.0f;
    }
    /* ----------------------------------------------------------------------------------------
     * Internal data */

    /**
       Get the current value of a parameter.
     */
    float getParameterValue(uint32_t index) const override {
        // same as before, ignore index check
        return fGain;
    }

    /**
       Change a parameter value.
     */
    void setParameterValue(uint32_t index, float value) override {
        // same as before, ignore index check
        fGain = value;
    }

    /* ----------------------------------------------------------------------------------------
     * Audio/MIDI Processing */
    void run(const float** inputs, float** outputs, uint32_t frames) override {
        // get the mono input and output
        const float* const in = inputs[0];
        /* */ float* const out = outputs[0];
        // apply gain against all samples
        for (uint32_t i = 0; i < frames; ++i)
            out[i] = in[i] * fGain;
    }
private:
    float fGain;
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

