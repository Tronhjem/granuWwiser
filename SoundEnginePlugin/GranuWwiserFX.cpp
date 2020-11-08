/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2020 Audiokinetic Inc.
*******************************************************************************/

#include "GranuWwiserFX.h"
#include "../GranuWwiserConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateGranuWwiserFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, GranuWwiserFX());
}

AK::IAkPluginParam* CreateGranuWwiserFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, GranuWwiserFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(GranuWwiserFX, AkPluginTypeEffect, GranuWwiserConfig::CompanyID, GranuWwiserConfig::PluginID)

GranuWwiserFX::GranuWwiserFX()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
    
}

GranuWwiserFX::~GranuWwiserFX()
{
    delete m_buffer;
}

AKRESULT GranuWwiserFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (GranuWwiserFXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;
    float secondsOfBufferTime = 2;
    m_buffer = new Buffer(int(secondsOfBufferTime * in_rFormat.uSampleRate));
    return AK_Success;
}

AKRESULT GranuWwiserFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT GranuWwiserFX::Reset()
{
    return AK_Success;
}

AKRESULT GranuWwiserFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

void GranuWwiserFX::Execute(AkAudioBuffer* io_pBuffer)
{
    const AkUInt32 uNumChannels = io_pBuffer->NumChannels();

    AkUInt16 uFramesProcessed;
    for (AkUInt32 i = 0; i < uNumChannels; ++i)
    {
        AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_pBuffer->GetChannel(i);
        
        m_writePointerBuffer = m_buffer->GetWritePointer(i);

        uFramesProcessed = 0;
        while (uFramesProcessed < io_pBuffer->uValidFrames)
        {
            m_windowSize = m_pParams->RTPC.fWindowSize;
            m_reSampleThreshold = m_pParams->RTPC.fSampleRetrigger;

            if (m_currentSampleCount[i] >= m_reSampleThreshold)
                m_currentSampleCount[i] = 0;

            // Write initial block to hold to the buffer
            if (m_currentSampleCount[i] < m_windowSize)
            {
                // Fade in
                if (m_currentSampleCount[i] <= m_fadeSize)
                    m_writePointerBuffer[uFramesProcessed] = pBuf[uFramesProcessed] * float(m_currentSampleCount[i] / m_fadeSize);
                // Fade out
                else if (m_currentSampleCount[i] >= m_windowSize - m_fadeSize)
                    m_writePointerBuffer[uFramesProcessed] = pBuf[uFramesProcessed] * float((m_windowSize - m_currentSampleCount[i]) / m_fadeSize);
                else
                    m_writePointerBuffer[uFramesProcessed] = pBuf[uFramesProcessed];
            }
            // play back held sample block from buffer until resample is hit.
            else if (m_currentSampleCount[i] > m_windowSize && m_currentSampleCount[i] < m_reSampleThreshold)
            {
                pBuf[uFramesProcessed] = m_writePointerBuffer[m_currentSampleCount[i] % m_windowSize];
            }

            m_currentSampleCount[i] += 1;
            ++uFramesProcessed;
        }
    }
}

AKRESULT GranuWwiserFX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
