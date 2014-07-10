/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2013, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TEncCu.h
    \brief    Coding Unit (CU) encoder class (header)
*/

#ifndef X265_TENCCU_H
#define X265_TENCCU_H

#define ANGULAR_MODE_ID 2
#define AMP_ID 3
#define INTER_MODES 4
#define INTRA_MODES 3

#include "common.h"
#include "TLibCommon/TComYuv.h"
#include "TLibCommon/TComPrediction.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/TComDataCU.h"
#include "shortyuv.h"

#include "entropy.h"
#include "TEncSearch.h"

//! \ingroup TLibEncoder
//! \{
struct StatisticLog
{
    uint64_t cntInter[4];
    uint64_t cntIntra[4];
    uint64_t cuInterDistribution[4][INTER_MODES];
    uint64_t cuIntraDistribution[4][INTRA_MODES];
    uint64_t cntIntraNxN;
    uint64_t cntSkipCu[4];
    uint64_t cntTotalCu[4];
    uint64_t totalCu;

    /* These states store the count of inter,intra and skip ctus within quad tree structure of each CU */
    uint32_t qTreeInterCnt[4];
    uint32_t qTreeIntraCnt[4];
    uint32_t qTreeSkipCnt[4];
    StatisticLog()
    {
        memset(this, 0, sizeof(StatisticLog));
    }
};

namespace x265 {
// private namespace

class Encoder;
class SBac;

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CU encoder class
class TEncCu
{
public:

    static const int MAX_PRED_TYPES = 6;

    TComDataCU* m_memPool;

    TComDataCU* m_interCU_2Nx2N[MAX_CU_DEPTH];
    TComDataCU* m_interCU_2NxN[MAX_CU_DEPTH];
    TComDataCU* m_interCU_Nx2N[MAX_CU_DEPTH];
    TComDataCU* m_intraInInterCU[MAX_CU_DEPTH];
    TComDataCU* m_mergeCU[MAX_CU_DEPTH];
    TComDataCU* m_bestMergeCU[MAX_CU_DEPTH];
    TComDataCU* m_bestCU[MAX_CU_DEPTH];      ///< Best CUs at each depth
    TComDataCU* m_tempCU[MAX_CU_DEPTH];      ///< Temporary CUs at each depth

    TComYuv**    m_bestPredYuv; ///< Best Prediction Yuv for each depth
    ShortYuv**   m_bestResiYuv; ///< Best Residual Yuv for each depth
    TComYuv**    m_bestRecoYuv; ///< Best Reconstruction Yuv for each depth

    TComYuv**    m_tmpPredYuv;  ///< Temporary Prediction Yuv for each depth
    ShortYuv**   m_tmpResiYuv;  ///< Temporary Residual Yuv for each depth
    TComYuv**    m_tmpRecoYuv;  ///< Temporary Reconstruction Yuv for each depth
    TComYuv**    m_modePredYuv[MAX_PRED_TYPES]; ///< Prediction buffers for inter, intra, rect(2) and merge
    TComYuv**    m_bestMergeRecoYuv;
    TComYuv**    m_origYuv;     ///< Original Yuv at each depth

    x265_param*  m_param;
    TEncSearch*  m_search;
    TComTrQuant* m_trQuant;
    RDCost*      m_rdCost;
    SBac*        m_sbacCoder;
    bool         m_bBitCounting;

    // RD SBac pointers
    SBac       (*m_rdSbacCoders)[CI_NUM];

    uint8_t      m_totalDepth;

    bool         m_bEncodeDQP;
    bool         m_CUTransquantBypass;

    StatisticLog  m_sliceTypeLog[3];
    StatisticLog* m_log;

    TEncCu();

    void init(Encoder* top);
    bool create(uint8_t totalDepth, uint32_t maxWidth);
    void destroy();
    void compressCU(TComDataCU* cu);
    void encodeCU(TComDataCU* cu, bool bIsCounting);

protected:

    void finishCU(TComDataCU* cu, uint32_t absPartIdx, uint32_t depth);
    void xCompressCU(TComDataCU*& outBestCU, TComDataCU*& outTempCU, uint32_t depth, bool bInsidePicture, PartSize parentSize = SIZE_NONE);
    void xCompressIntraCU(TComDataCU*& outBestCU, TComDataCU*& outTempCU, uint32_t depth, bool bInsidePicture);
    void xCompressInterCU(TComDataCU*& outBestCU, TComDataCU*& outTempCU, TComDataCU* cu, uint32_t depth, bool bInsidePicture, uint32_t partitionIndex, uint8_t minDepth);
    void xEncodeCU(TComDataCU* cu, uint32_t absPartIdx, uint32_t depth, bool bInsidePicture);
    void xCheckBestMode(TComDataCU*& outBestCU, TComDataCU*& outTempCU, uint32_t depth);

    void xCheckRDCostMerge2Nx2N(TComDataCU*& outBestCU, TComDataCU*& outTempCU, bool *earlyDetectionSkipMode,
                                TComYuv*& outBestPredYuv, TComYuv*& rpcYuvReconBest);
    void xComputeCostIntraInInter(TComDataCU* cu, PartSize partSize);
    void xCheckRDCostInter(TComDataCU*& outBestCU, TComDataCU*& outTempCU, PartSize partSize, bool bUseMRG = false);
    void xComputeCostInter(TComDataCU* outTempCU, TComYuv* outPredYUV, PartSize partSize, bool bUseMRG = false);
    void xComputeCostMerge2Nx2N(TComDataCU*& outBestCU, TComDataCU*& outTempCU, TComYuv*& bestPredYuv, TComYuv*& tmpPredYuv);
    void xEncodeIntraInInter(TComDataCU* cu, TComYuv* fencYuv, TComYuv* predYuv, ShortYuv* outResiYuv, TComYuv* outReconYuv);
    void encodeResidue(TComDataCU* lcu, TComDataCU* cu, uint32_t absPartIdx, uint8_t depth);
    void xCheckRDCostIntra(TComDataCU*& outBestCU, TComDataCU*& outTempCU, PartSize partSize);
    void xCheckRDCostIntraInInter(TComDataCU*& outBestCU, TComDataCU*& outTempCU, PartSize partSize);
    void xCheckDQP(TComDataCU* cu);

    void xCopyYuv2Pic(Frame* outPic, uint32_t cuAddr, uint32_t absPartIdx, uint32_t depth);
    void xCopyYuv2Tmp(uint32_t uhPartUnitIdx, uint32_t depth);

    void deriveTestModeAMP(TComDataCU* bestCU, PartSize parentSize, bool &bTestAMP_Hor, bool &bTestAMP_Ver,
                           bool &bTestMergeAMP_Hor, bool &bTestMergeAMP_Ver);

    void xFillOrigYUVBuffer(TComDataCU* outCU, TComYuv* origYuv);
};
}
//! \}

#endif // ifndef X265_TENCCU_H
