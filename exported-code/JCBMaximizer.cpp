#include "JCBMaximizer.h"

namespace JCBMaximizer {

/****************************************************************************************
Copyright (c) 2023 Cycling '74

The code that Max generates automatically and that end users are capable of
exporting and using, and any associated documentation files (the “Software”)
is a work of authorship for which Cycling '74 is the author and owner for
copyright purposes.

This Software is dual-licensed either under the terms of the Cycling '74
License for Max-Generated Code for Export, or alternatively under the terms
of the General Public License (GPL) Version 3. You may use the Software
according to either of these licenses as it is most appropriate for your
project on a case-by-case basis (proprietary or not).

A) Cycling '74 License for Max-Generated Code for Export

A license is hereby granted, free of charge, to any person obtaining a copy
of the Software (“Licensee”) to use, copy, modify, merge, publish, and
distribute copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The Software is licensed to Licensee for all uses that do not include the sale,
sublicensing, or commercial distribution of software that incorporates this
source code. This means that the Licensee is free to use this software for
educational, research, and prototyping purposes, to create musical or other
creative works with software that incorporates this source code, or any other
use that does not constitute selling software that makes use of this source
code. Commercial distribution also includes the packaging of free software with
other paid software, hardware, or software-provided commercial services.

For entities with UNDER 200k USD in annual revenue or funding, a license is hereby
granted, free of charge, for the sale, sublicensing, or commercial distribution
of software that incorporates this source code, for as long as the entity's
annual revenue remains below 200k USD annual revenue or funding.

For entities with OVER 200k USD in annual revenue or funding interested in the
sale, sublicensing, or commercial distribution of software that incorporates
this source code, please send inquiries to licensing (at) cycling74.com.

The above copyright notice and this license shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Please see
https://support.cycling74.com/hc/en-us/articles/360050779193-Gen-Code-Export-Licensing-FAQ
for additional information

B) General Public License Version 3 (GPLv3)
Details of the GPLv3 license can be found at: https://www.gnu.org/licenses/gpl-3.0.html
****************************************************************************************/

// global noise generator
Noise noise;
[[maybe_unused]] static const int GENLIB_LOOPCOUNT_BAIL = 100000;


// The State struct contains all the state and procedures for the gendsp kernel
typedef struct State {
	CommonState __commonstate;
	DCBlock __m_dcblock_44;
	Delay m_rightInputDelay_2;
	Delay m_leftInputDelay_3;
	Delay m_delayDetectRight_4;
	Delay m_rmsDelay_1;
	Delay m_delayLeft_7;
	Delay m_delayRight_6;
	Delay m_delayDetectLeft_5;
	int __exception;
	int vectorsize;
	t_sample m_n_LOOKAHEAD_31;
	t_sample m_o_DCFILT_32;
	t_sample m_m_AUTOREL_33;
	t_sample m_smoothedGain_30;
	t_sample m_smoothedBypass_28;
	t_sample m_smoothedCeiling_29;
	t_sample m_smoothedMakeup_27;
	t_sample m_envelopeFollower_26;
	t_sample m_l_DETECT_34;
	t_sample m_j_TRIM_36;
	t_sample m_a_GAIN_41;
	t_sample m_b_CELLING_42;
	t_sample m_k_DELTA_35;
	t_sample m_e_REL_40;
	t_sample m_h_BYPASS_38;
	t_sample m_g_DITHER_39;
	t_sample m_i_MAKEUP_37;
	t_sample samplerate;
	t_sample m_gainReduction_25;
	t_sample m_trimHistory_23;
	t_sample m_hpfPrevInL_12;
	t_sample m_deltaRmsStateR_13;
	t_sample m_deltaRmsStateL_14;
	t_sample m_hpfPrevInR_11;
	t_sample m_hpfStateR_9;
	t_sample m_hpfStateL_10;
	t_sample m_dcFiltHistory_8;
	t_sample m_lookaheadHistory_24;
	t_sample m_progRmsStateR_15;
	t_sample m_rmsSum_17;
	t_sample m_d_ATK_43;
	t_sample m_deltaHistory_22;
	t_sample m_progRmsStateL_16;
	t_sample m_autoReleaseHistory_20;
	t_sample m_prevDetection_18;
	t_sample m_transientDetector_19;
	t_sample m_detectHistory_21;
	// re-initialize all member variables;
	inline void reset(t_param __sr, int __vs) {
		__exception = 0;
		vectorsize = __vs;
		samplerate = __sr;
		m_rmsDelay_1.reset("m_rmsDelay_1", ((int)500));
		m_rightInputDelay_2.reset("m_rightInputDelay_2", samplerate);
		m_leftInputDelay_3.reset("m_leftInputDelay_3", samplerate);
		m_delayDetectRight_4.reset("m_delayDetectRight_4", samplerate);
		m_delayDetectLeft_5.reset("m_delayDetectLeft_5", samplerate);
		m_delayRight_6.reset("m_delayRight_6", samplerate);
		m_delayLeft_7.reset("m_delayLeft_7", samplerate);
		m_dcFiltHistory_8 = ((int)0);
		m_hpfStateR_9 = ((int)0);
		m_hpfStateL_10 = ((int)0);
		m_hpfPrevInR_11 = ((int)0);
		m_hpfPrevInL_12 = ((int)0);
		m_deltaRmsStateR_13 = ((int)0);
		m_deltaRmsStateL_14 = ((int)0);
		m_progRmsStateR_15 = ((int)0);
		m_progRmsStateL_16 = ((int)0);
		m_rmsSum_17 = ((int)0);
		m_prevDetection_18 = ((int)0);
		m_transientDetector_19 = ((int)0);
		m_autoReleaseHistory_20 = ((int)0);
		m_detectHistory_21 = ((int)0);
		m_deltaHistory_22 = ((int)0);
		m_trimHistory_23 = ((int)0);
		m_lookaheadHistory_24 = ((int)0);
		m_gainReduction_25 = ((int)0);
		m_envelopeFollower_26 = ((int)0);
		m_smoothedMakeup_27 = ((int)0);
		m_smoothedBypass_28 = ((int)0);
		m_smoothedCeiling_29 = ((int)0);
		m_smoothedGain_30 = ((int)0);
		m_n_LOOKAHEAD_31 = 0;
		m_o_DCFILT_32 = 0;
		m_m_AUTOREL_33 = 0;
		m_l_DETECT_34 = 0;
		m_k_DELTA_35 = 0;
		m_j_TRIM_36 = 0;
		m_i_MAKEUP_37 = 0;
		m_h_BYPASS_38 = 0;
		m_g_DITHER_39 = 0;
		m_e_REL_40 = 200;
		m_a_GAIN_41 = 0;
		m_b_CELLING_42 = -0.3;
		m_d_ATK_43 = 100;
		__m_dcblock_44.reset();
		genlib_reset_complete(this);
		
	};
	// the signal processing routine;
	inline int perform(t_sample ** __ins, t_sample ** __outs, int __n) {
		vectorsize = __n;
		const t_sample * __in1 = __ins[0];
		const t_sample * __in2 = __ins[1];
		t_sample * __out1 = __outs[0];
		t_sample * __out2 = __outs[1];
		t_sample * __out3 = __outs[2];
		t_sample * __out4 = __outs[3];
		t_sample * __out5 = __outs[4];
		if (__exception) {
			return __exception;
			
		} else if (( (__in1 == 0) || (__in2 == 0) || (__out1 == 0) || (__out2 == 0) || (__out3 == 0) || (__out4 == 0) || (__out5 == 0) )) {
			__exception = GENLIB_ERR_NULL_BUFFER;
			return __exception;
			
		};
		t_sample ceilingLinear = dbtoa(m_b_CELLING_42);
		t_sample maxb_45 = floor(((((int)3) * samplerate) * ((t_sample)0.001)));
		t_sample rmsWindowSize = ((((int)1) < maxb_45) ? maxb_45 : ((int)1));
		t_sample rmsWindowInv = safediv(((int)1), rmsWindowSize);
		t_sample attackTime = ((m_d_ATK_43 * ((t_sample)0.001)) * samplerate);
		t_sample attackCoeff = exp(safediv(((t_sample)-0.99967234081321), attackTime));
		t_sample ceilingNegative = (ceilingLinear * ((int)-1));
		t_sample ceilingPositive = (ceilingLinear * ((int)1));
		int max_49 = (samplerate - ((int)1));
		t_sample choice_54 = int(m_g_DITHER_39);
		t_sample choice_55 = int(m_g_DITHER_39);
		t_sample dc_r = exp(safediv(((t_sample)-75.398223686155), samplerate));
		t_sample maxb_56 = floor((((t_sample)0.02) * samplerate));
		t_sample rmsTimeSamples = ((((int)1) < maxb_56) ? maxb_56 : ((int)1));
		t_sample rmsCoeff = exp(safediv(((t_sample)-0.99967234081321), rmsTimeSamples));
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			const t_sample in2 = (*(__in2++));
			t_sample smoothedTrim = ((m_trimHistory_23 * ((t_sample)0.999)) + (m_j_TRIM_36 * ((t_sample)0.001)));
			m_trimHistory_23 = fixdenorm(smoothedTrim);
			t_sample trimLinear = dbtoa(smoothedTrim);
			m_smoothedGain_30 = ((m_smoothedGain_30 * ((t_sample)0.999)) + ((-m_a_GAIN_41) * ((t_sample)0.001)));
			m_smoothedGain_30 = fixdenorm(m_smoothedGain_30);
			t_sample thresholdLinear = safediv(((int)1), safepow(((int)10), (m_smoothedGain_30 * ((t_sample)0.05))));
			m_smoothedCeiling_29 = ((m_smoothedCeiling_29 * ((t_sample)0.999)) + (m_b_CELLING_42 * ((t_sample)0.001)));
			m_smoothedCeiling_29 = fixdenorm(m_smoothedCeiling_29);
			m_smoothedBypass_28 = ((m_smoothedBypass_28 * ((t_sample)0.999)) + (m_h_BYPASS_38 * ((t_sample)0.001)));
			m_smoothedBypass_28 = fixdenorm(m_smoothedBypass_28);
			t_sample wetAmount = (((int)1) - m_smoothedBypass_28);
			m_smoothedMakeup_27 = ((m_smoothedMakeup_27 * ((t_sample)0.999)) + (m_i_MAKEUP_37 * ((t_sample)0.001)));
			m_smoothedMakeup_27 = fixdenorm(m_smoothedMakeup_27);
			t_sample makeupLinear = dbtoa(m_smoothedMakeup_27);
			t_sample smoothedDelta = ((m_deltaHistory_22 * ((t_sample)0.999)) + (m_k_DELTA_35 * ((t_sample)0.001)));
			m_deltaHistory_22 = fixdenorm(smoothedDelta);
			t_sample smoothedDetect = ((m_detectHistory_21 * ((t_sample)0.999)) + (m_l_DETECT_34 * ((t_sample)0.001)));
			m_detectHistory_21 = fixdenorm(smoothedDetect);
			t_sample smoothedAutoRelease = ((m_autoReleaseHistory_20 * ((t_sample)0.999)) + (m_m_AUTOREL_33 * ((t_sample)0.001)));
			m_autoReleaseHistory_20 = fixdenorm(smoothedAutoRelease);
			t_sample smoothedDCFILT = ((m_dcFiltHistory_8 * ((t_sample)0.95)) + (m_o_DCFILT_32 * ((t_sample)0.05)));
			m_dcFiltHistory_8 = fixdenorm(smoothedDCFILT);
			t_sample leftTrimmed = (in1 * trimLinear);
			t_sample rightTrimmed = (in2 * trimLinear);
			t_sample rightScaled = ((ceilingLinear * rightTrimmed) * thresholdLinear);
			t_sample leftScaled = ((ceilingLinear * leftTrimmed) * thresholdLinear);
			m_delayDetectLeft_5.write(leftScaled);
			m_delayDetectRight_4.write(rightScaled);
			m_delayLeft_7.write(leftTrimmed);
			m_delayRight_6.write(rightTrimmed);
			m_leftInputDelay_3.write(in1);
			m_rightInputDelay_2.write(in2);
			t_sample averageSignal = ((leftScaled + rightScaled) * ((t_sample)0.5));
			t_sample detIn = __m_dcblock_44(averageSignal);
			t_sample peakDetection = fabs(detIn);
			t_sample signalSquared = (detIn * detIn);
			m_rmsDelay_1.write(signalSquared);
			t_sample oldestSquared = m_rmsDelay_1.read_step(rmsWindowSize);
			t_sample rmsSumNew = ((signalSquared + m_rmsSum_17) - oldestSquared);
			t_sample rmsSumClipped = ((((int)0) < rmsSumNew) ? rmsSumNew : ((int)0));
			t_sample rmsDetection = sqrt((rmsSumClipped * rmsWindowInv));
			m_rmsSum_17 = fixdenorm(rmsSumClipped);
			t_sample mix_1537 = (peakDetection + (smoothedDetect * (rmsDetection - peakDetection)));
			t_sample detectionSignal = mix_1537;
			t_sample finalReleaseTime = m_e_REL_40;
			if ((smoothedAutoRelease > ((t_sample)0.01))) {
				t_sample signalChange = fabs((detectionSignal - m_prevDetection_18));
				m_prevDetection_18 = detectionSignal;
				t_sample maxb_46 = (detectionSignal * ((t_sample)0.1));
				t_sample relativeThreshold = ((((t_sample)0.001) < maxb_46) ? maxb_46 : ((t_sample)0.001));
				int cond_47 = (signalChange > relativeThreshold);
				int isTransient = (cond_47 ? ((int)1) : ((int)0));
				m_transientDetector_19 = ((m_transientDetector_19 * ((t_sample)0.99)) + (isTransient * ((t_sample)0.01)));
				t_sample mix_1538 = (((int)150) + (m_transientDetector_19 * ((int)-145)));
				t_sample autoRelease = mix_1538;
				t_sample mix_1539 = (m_e_REL_40 + (smoothedAutoRelease * (autoRelease - m_e_REL_40)));
				finalReleaseTime = mix_1539;
				
			};
			t_sample releaseTime = ((finalReleaseTime * ((t_sample)0.001)) * samplerate);
			t_sample releaseCoeff = exp(safediv(((t_sample)-0.99967234081321), releaseTime));
			t_sample maxb_48 = (m_envelopeFollower_26 * releaseCoeff);
			m_envelopeFollower_26 = ((detectionSignal < maxb_48) ? maxb_48 : detectionSignal);
			m_gainReduction_25 = ((m_gainReduction_25 * attackCoeff) + (m_envelopeFollower_26 * (((int)1) - attackCoeff)));
			m_gainReduction_25 = ((m_gainReduction_25 < ((t_sample)1e-06)) ? ((t_sample)1e-06) : m_gainReduction_25);
			m_gainReduction_25 = fixdenorm(m_gainReduction_25);
			t_sample gainReductionDb = atodb(m_gainReduction_25);
			m_envelopeFollower_26 = fixdenorm(m_envelopeFollower_26);
			t_sample limitedDb = softkneeLimiter_d_d_i(gainReductionDb, m_smoothedCeiling_29, ((int)0));
			t_sample gainReductionAmount = (limitedDb - gainReductionDb);
			t_sample gainReductionAmount_1506 = ((gainReductionAmount < ((int)-144)) ? ((int)-144) : gainReductionAmount);
			t_sample gainReductionLinear = dbtoa(gainReductionAmount_1506);
			t_sample smoothedLookahead = ((m_lookaheadHistory_24 * ((t_sample)0.99)) + (m_n_LOOKAHEAD_31 * ((t_sample)0.01)));
			m_lookaheadHistory_24 = fixdenorm(smoothedLookahead);
			t_sample lookaheadSamples = round((smoothedLookahead * (samplerate * 0.001)));
			t_sample lh = ((lookaheadSamples <= ((int)0)) ? ((int)0) : ((lookaheadSamples >= max_49) ? max_49 : lookaheadSamples));
			t_sample delayedLeft = m_delayLeft_7.read_step(lh);
			t_sample delayedRight = m_delayRight_6.read_step(lh);
			t_sample delayedLeftOriginal = m_leftInputDelay_3.read_step(lh);
			t_sample delayedRightOriginal = m_rightInputDelay_2.read_step(lh);
			t_sample programL = delayedLeft;
			t_sample programR = delayedRight;
			t_sample programDrivenL = (programL * thresholdLinear);
			t_sample programDrivenR = (programR * thresholdLinear);
			t_sample maxa_50 = fabs(programDrivenL);
			t_sample minb_51 = safediv(ceilingLinear, ((maxa_50 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : maxa_50));
			t_sample g_br_L = ((minb_51 < ((int)1)) ? minb_51 : ((int)1));
			t_sample maxa_52 = fabs(programDrivenR);
			t_sample minb_53 = safediv(ceilingLinear, ((maxa_52 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : maxa_52));
			t_sample g_br_R = ((minb_53 < ((int)1)) ? minb_53 : ((int)1));
			t_sample finalGain_L = ((gainReductionLinear < g_br_L) ? gainReductionLinear : g_br_L);
			t_sample finalGain_R = ((gainReductionLinear < g_br_R) ? gainReductionLinear : g_br_R);
			t_sample noiseL1 = ((noise() * ((t_sample)1.5258789062e-05)) * ((t_sample)0.5));
			t_sample noiseL2 = ((noise() * ((t_sample)1.5258789062e-05)) * ((t_sample)0.5));
			t_sample ditherTPDF_L = (noiseL1 + noiseL2);
			t_sample noiseR1 = ((noise() * ((t_sample)1.5258789062e-05)) * ((t_sample)0.5));
			t_sample noiseR2 = ((noise() * ((t_sample)1.5258789062e-05)) * ((t_sample)0.5));
			t_sample ditherTPDF_R = (noiseR1 + noiseR2);
			t_sample ditherGatedL = (((choice_54 >= 1) && (choice_54 < 2)) ? ditherTPDF_L : 0);
			t_sample ditherGatedR = (((choice_55 >= 1) && (choice_55 < 2)) ? ditherTPDF_R : 0);
			t_sample leftProcessed = (programDrivenL * finalGain_L);
			t_sample hpfOutL_pre = ((leftProcessed - m_hpfPrevInL_12) + (dc_r * m_hpfStateL_10));
			m_hpfPrevInL_12 = leftProcessed;
			m_hpfStateL_10 = hpfOutL_pre;
			t_sample mix_1540 = (leftProcessed + (smoothedDCFILT * (hpfOutL_pre - leftProcessed)));
			t_sample leftPre = mix_1540;
			t_sample leftClipped = ((leftPre <= ceilingNegative) ? ceilingNegative : ((leftPre >= ceilingPositive) ? ceilingPositive : leftPre));
			t_sample leftDeltaRaw = (programDrivenL - leftClipped);
			t_sample leftWithMakeup = (leftClipped * makeupLinear);
			t_sample sub_1542 = (((int)1) - smoothedDelta);
			t_sample mix_1541 = (leftClipped + (sub_1542 * (leftWithMakeup - leftClipped)));
			t_sample rightProcessed = (programDrivenR * finalGain_R);
			t_sample hpfOutR_pre = ((rightProcessed - m_hpfPrevInR_11) + (dc_r * m_hpfStateR_9));
			m_hpfPrevInR_11 = rightProcessed;
			m_hpfStateR_9 = hpfOutR_pre;
			t_sample mix_1543 = (rightProcessed + (smoothedDCFILT * (hpfOutR_pre - rightProcessed)));
			t_sample rightPre = mix_1543;
			t_sample rightClipped = ((rightPre <= ceilingNegative) ? ceilingNegative : ((rightPre >= ceilingPositive) ? ceilingPositive : rightPre));
			t_sample rightDeltaRaw = (programDrivenR - rightClipped);
			t_sample progLSq = (programDrivenL * programDrivenL);
			t_sample progRSq = (programDrivenR * programDrivenR);
			m_progRmsStateL_16 = ((m_progRmsStateL_16 * rmsCoeff) + (progLSq * (((int)1) - rmsCoeff)));
			m_progRmsStateR_15 = ((m_progRmsStateR_15 * rmsCoeff) + (progRSq * (((int)1) - rmsCoeff)));
			t_sample progRmsL = sqrt(((m_progRmsStateL_16 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : m_progRmsStateL_16));
			t_sample progRmsR = sqrt(((m_progRmsStateR_15 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : m_progRmsStateR_15));
			t_sample deltaLSq = (leftDeltaRaw * leftDeltaRaw);
			t_sample deltaRSq = (rightDeltaRaw * rightDeltaRaw);
			m_deltaRmsStateL_14 = ((m_deltaRmsStateL_14 * rmsCoeff) + (deltaLSq * (((int)1) - rmsCoeff)));
			m_deltaRmsStateR_13 = ((m_deltaRmsStateR_13 * rmsCoeff) + (deltaRSq * (((int)1) - rmsCoeff)));
			t_sample deltaRmsL = sqrt(((m_deltaRmsStateL_14 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : m_deltaRmsStateL_14));
			t_sample deltaRmsR = sqrt(((m_deltaRmsStateR_13 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : m_deltaRmsStateR_13));
			t_sample minb_57 = safediv(progRmsL, ((deltaRmsL < ((t_sample)1e-12)) ? ((t_sample)1e-12) : deltaRmsL));
			t_sample normGainL = ((minb_57 < ((int)1)) ? minb_57 : ((int)1));
			t_sample minb_58 = safediv(progRmsR, ((deltaRmsR < ((t_sample)1e-12)) ? ((t_sample)1e-12) : deltaRmsR));
			t_sample normGainR = ((minb_58 < ((int)1)) ? minb_58 : ((int)1));
			t_sample leftDeltaNorm = (leftDeltaRaw * normGainL);
			t_sample rightDeltaNorm = (rightDeltaRaw * normGainR);
			t_sample grLinAvg = ((finalGain_L + finalGain_R) * ((t_sample)0.5));
			t_sample grDb = atodb(((grLinAvg < ((t_sample)1e-12)) ? ((t_sample)1e-12) : grLinAvg));
			t_sample v_59 = (((-grDb) - ((int)6)) * ((t_sample)0.083333333333333));
			t_sample t = ((v_59 <= ((int)0)) ? ((int)0) : ((v_59 >= ((int)1)) ? ((int)1) : v_59));
			t_sample mix_1544 = (((int)1) + (t * ((t_sample)-0.65)));
			t_sample leftDeltaNorm_1507 = (leftDeltaNorm * mix_1544);
			t_sample rightDeltaNorm_1508 = (rightDeltaNorm * mix_1544);
			t_sample maxa_60 = fabs(leftDeltaNorm_1507);
			t_sample minb_61 = safediv(ceilingLinear, ((maxa_60 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : maxa_60));
			t_sample deltaPeakGuardL = ((minb_61 < ((int)1)) ? minb_61 : ((int)1));
			t_sample maxa_62 = fabs(rightDeltaNorm_1508);
			t_sample minb_63 = safediv(ceilingLinear, ((maxa_62 < ((t_sample)1e-12)) ? ((t_sample)1e-12) : maxa_62));
			t_sample deltaPeakGuardR = ((minb_63 < ((int)1)) ? minb_63 : ((int)1));
			t_sample leftDeltaSafe = (leftDeltaNorm_1507 * deltaPeakGuardL);
			t_sample rightDeltaSafe = (rightDeltaNorm_1508 * deltaPeakGuardR);
			t_sample rightWithMakeup = (rightClipped * makeupLinear);
			t_sample sub_1546 = (((int)1) - smoothedDelta);
			t_sample mix_1545 = (rightClipped + (sub_1546 * (rightWithMakeup - rightClipped)));
			t_sample mix_1547 = (mix_1541 + (smoothedDelta * (leftDeltaSafe - mix_1541)));
			t_sample mix_1548 = (mix_1545 + (smoothedDelta * (rightDeltaSafe - mix_1545)));
			t_sample mix_1549 = (delayedLeftOriginal + (wetAmount * (mix_1547 - delayedLeftOriginal)));
			t_sample mix_1550 = (delayedRightOriginal + (wetAmount * (mix_1548 - delayedRightOriginal)));
			t_sample expr_1512 = (mix_1550 + ditherGatedR);
			t_sample expr_1511 = (mix_1549 + ditherGatedL);
			t_sample finalGainDisplay = ((((gainReductionLinear < g_br_L) ? gainReductionLinear : g_br_L) + ((gainReductionLinear < g_br_R) ? gainReductionLinear : g_br_R)) * ((t_sample)0.5));
			t_sample mix_1551 = (((int)1) + (wetAmount * (finalGainDisplay - ((int)1))));
			t_sample gainReductionOutput = ((mix_1551 <= ((int)0)) ? ((int)0) : ((mix_1551 >= ((int)1)) ? ((int)1) : mix_1551));
			t_sample out5 = rightTrimmed;
			t_sample out4 = leftTrimmed;
			t_sample out1 = expr_1511;
			t_sample out2 = expr_1512;
			t_sample out3 = gainReductionOutput;
			m_rmsDelay_1.step();
			m_rightInputDelay_2.step();
			m_leftInputDelay_3.step();
			m_delayDetectRight_4.step();
			m_delayDetectLeft_5.step();
			m_delayRight_6.step();
			m_delayLeft_7.step();
			// assign results to output buffer;
			(*(__out1++)) = out1;
			(*(__out2++)) = out2;
			(*(__out3++)) = out3;
			(*(__out4++)) = out4;
			(*(__out5++)) = out5;
			
		};
		return __exception;
		
	};
	inline void set_n_LOOKAHEAD(t_param _value) {
		m_n_LOOKAHEAD_31 = (_value < 0 ? 0 : (_value > 5 ? 5 : _value));
	};
	inline void set_o_DCFILT(t_param _value) {
		m_o_DCFILT_32 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_m_AUTOREL(t_param _value) {
		m_m_AUTOREL_33 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_l_DETECT(t_param _value) {
		m_l_DETECT_34 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_k_DELTA(t_param _value) {
		m_k_DELTA_35 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_j_TRIM(t_param _value) {
		m_j_TRIM_36 = (_value < -12 ? -12 : (_value > 12 ? 12 : _value));
	};
	inline void set_i_MAKEUP(t_param _value) {
		m_i_MAKEUP_37 = (_value < -12 ? -12 : (_value > 12 ? 12 : _value));
	};
	inline void set_h_BYPASS(t_param _value) {
		m_h_BYPASS_38 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_g_DITHER(t_param _value) {
		m_g_DITHER_39 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_e_REL(t_param _value) {
		m_e_REL_40 = (_value < 1 ? 1 : (_value > 1000 ? 1000 : _value));
	};
	inline void set_a_GAIN(t_param _value) {
		m_a_GAIN_41 = (_value < 0 ? 0 : (_value > 24 ? 24 : _value));
	};
	inline void set_b_CELLING(t_param _value) {
		m_b_CELLING_42 = (_value < -60 ? -60 : (_value > 0 ? 0 : _value));
	};
	inline void set_d_ATK(t_param _value) {
		m_d_ATK_43 = (_value < 0.01 ? 0.01 : (_value > 750 ? 750 : _value));
	};
	inline t_sample softkneeLimiter_d_d_i(t_sample xg, t_sample threshold, int kneeWidth) {
		t_sample ret = ((int)0);
		if (((((int)2) * (xg - threshold)) < (kneeWidth * (-((int)1))))) {
			ret = xg;
			
		} else {
			if (((((int)2) * fabs((xg - threshold))) <= kneeWidth)) {
				ret = (xg - safediv(safepow(((xg - threshold) - (kneeWidth * ((t_sample)0.5))), ((int)2)), (((int)2) * kneeWidth)));
				
			} else {
				if (((((int)2) * (xg - threshold)) > kneeWidth)) {
					ret = threshold;
					
				};
				
			};
			
		};
		return ret;
		
	};
	
} State;


///
///	Configuration for the genlib API
///

/// Number of signal inputs and outputs

int gen_kernel_numins = 2;
int gen_kernel_numouts = 5;

int num_inputs() { return gen_kernel_numins; }
int num_outputs() { return gen_kernel_numouts; }
int num_params() { return 13; }

/// Assistive lables for the signal inputs and outputs

const char *gen_kernel_innames[] = { "in1", "in2" };
const char *gen_kernel_outnames[] = { "out1", "out2", "out3", "out4", "out5" };

/// Invoke the signal process of a State object

int perform(CommonState *cself, t_sample **ins, long numins, t_sample **outs, long numouts, long n) {
	State* self = (State *)cself;
	return self->perform(ins, outs, n);
}

/// Reset all parameters and stateful operators of a State object

void reset(CommonState *cself) {
	State* self = (State *)cself;
	self->reset(cself->sr, cself->vs);
}

/// Set a parameter of a State object

void setparameter(CommonState *cself, long index, t_param value, void *ref) {
	State *self = (State *)cself;
	switch (index) {
		case 0: self->set_a_GAIN(value); break;
		case 1: self->set_b_CELLING(value); break;
		case 2: self->set_d_ATK(value); break;
		case 3: self->set_e_REL(value); break;
		case 4: self->set_g_DITHER(value); break;
		case 5: self->set_h_BYPASS(value); break;
		case 6: self->set_i_MAKEUP(value); break;
		case 7: self->set_j_TRIM(value); break;
		case 8: self->set_k_DELTA(value); break;
		case 9: self->set_l_DETECT(value); break;
		case 10: self->set_m_AUTOREL(value); break;
		case 11: self->set_n_LOOKAHEAD(value); break;
		case 12: self->set_o_DCFILT(value); break;
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		case 0: *value = self->m_a_GAIN_41; break;
		case 1: *value = self->m_b_CELLING_42; break;
		case 2: *value = self->m_d_ATK_43; break;
		case 3: *value = self->m_e_REL_40; break;
		case 4: *value = self->m_g_DITHER_39; break;
		case 5: *value = self->m_h_BYPASS_38; break;
		case 6: *value = self->m_i_MAKEUP_37; break;
		case 7: *value = self->m_j_TRIM_36; break;
		case 8: *value = self->m_k_DELTA_35; break;
		case 9: *value = self->m_l_DETECT_34; break;
		case 10: *value = self->m_m_AUTOREL_33; break;
		case 11: *value = self->m_n_LOOKAHEAD_31; break;
		case 12: *value = self->m_o_DCFILT_32; break;
		
		default: break;
	}
}

/// Get the name of a parameter of a State object

const char *getparametername(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].name;
	}
	return 0;
}

/// Get the minimum value of a parameter of a State object

t_param getparametermin(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].outputmin;
	}
	return 0;
}

/// Get the maximum value of a parameter of a State object

t_param getparametermax(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].outputmax;
	}
	return 0;
}

/// Get parameter of a State object has a minimum and maximum value

char getparameterhasminmax(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].hasminmax;
	}
	return 0;
}

/// Get the units of a parameter of a State object

const char *getparameterunits(CommonState *cself, long index) {
	if (index >= 0 && index < cself->numparams) {
		return cself->params[index].units;
	}
	return 0;
}

/// Get the size of the state of all parameters of a State object

size_t getstatesize(CommonState *cself) {
	return genlib_getstatesize(cself, &getparameter);
}

/// Get the state of all parameters of a State object

short getstate(CommonState *cself, char *state) {
	return genlib_getstate(cself, state, &getparameter);
}

/// set the state of all parameters of a State object

short setstate(CommonState *cself, const char *state) {
	return genlib_setstate(cself, state, &setparameter);
}

/// Allocate and configure a new State object and it's internal CommonState:

void *create(t_param sr, long vs) {
	State *self = new State;
	self->reset(sr, vs);
	ParamInfo *pi;
	self->__commonstate.inputnames = gen_kernel_innames;
	self->__commonstate.outputnames = gen_kernel_outnames;
	self->__commonstate.numins = gen_kernel_numins;
	self->__commonstate.numouts = gen_kernel_numouts;
	self->__commonstate.sr = sr;
	self->__commonstate.vs = vs;
	self->__commonstate.params = (ParamInfo *)genlib_sysmem_newptr(13 * sizeof(ParamInfo));
	self->__commonstate.numparams = 13;
	// initialize parameter 0 ("m_a_GAIN_41")
	pi = self->__commonstate.params + 0;
	pi->name = "a_GAIN";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_a_GAIN_41;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 24;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 1 ("m_b_CELLING_42")
	pi = self->__commonstate.params + 1;
	pi->name = "b_CELLING";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_b_CELLING_42;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -60;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 2 ("m_d_ATK_43")
	pi = self->__commonstate.params + 2;
	pi->name = "d_ATK";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_d_ATK_43;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.01;
	pi->outputmax = 750;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 3 ("m_e_REL_40")
	pi = self->__commonstate.params + 3;
	pi->name = "e_REL";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_e_REL_40;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 1;
	pi->outputmax = 1000;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 4 ("m_g_DITHER_39")
	pi = self->__commonstate.params + 4;
	pi->name = "g_DITHER";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_g_DITHER_39;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_h_BYPASS_38")
	pi = self->__commonstate.params + 5;
	pi->name = "h_BYPASS";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_h_BYPASS_38;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 6 ("m_i_MAKEUP_37")
	pi = self->__commonstate.params + 6;
	pi->name = "i_MAKEUP";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_i_MAKEUP_37;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -12;
	pi->outputmax = 12;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_j_TRIM_36")
	pi = self->__commonstate.params + 7;
	pi->name = "j_TRIM";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_j_TRIM_36;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -12;
	pi->outputmax = 12;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 8 ("m_k_DELTA_35")
	pi = self->__commonstate.params + 8;
	pi->name = "k_DELTA";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_k_DELTA_35;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_l_DETECT_34")
	pi = self->__commonstate.params + 9;
	pi->name = "l_DETECT";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_l_DETECT_34;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 10 ("m_m_AUTOREL_33")
	pi = self->__commonstate.params + 10;
	pi->name = "m_AUTOREL";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_m_AUTOREL_33;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 11 ("m_n_LOOKAHEAD_31")
	pi = self->__commonstate.params + 11;
	pi->name = "n_LOOKAHEAD";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_n_LOOKAHEAD_31;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 5;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 12 ("m_o_DCFILT_32")
	pi = self->__commonstate.params + 12;
	pi->name = "o_DCFILT";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_o_DCFILT_32;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	
	return self;
}

/// Release all resources and memory used by a State object:

void destroy(CommonState *cself) {
	State *self = (State *)cself;
	genlib_sysmem_freeptr(cself->params);
		
	delete self;
}


} // JCBMaximizer::
