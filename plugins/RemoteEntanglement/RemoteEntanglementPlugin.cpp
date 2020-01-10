/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   RemoteEntanglement.cpp
 * Author: my5t3ry
 * 
 * Created on 10. Januar 2020, 05:09
 */

#include "RemoteEntanglementPlugin.hpp"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------


Plugin* createPlugin() {
    return new RemoteEntanglementPlugin();
}

END_NAMESPACE_DISTRHO
