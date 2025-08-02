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
static const int GENLIB_LOOPCOUNT_BAIL = 100000;


// The State struct contains all the state and procedures for the gendsp kernel
typedef struct State {
	CommonState __commonstate;
	Delay m_delayDetectLeft_5;
	Delay m_rightInputDelay_2;
	Delay m_leftInputDelay_3;
	Delay m_rmsDelay_1;
	Delay m_delayLeft_7;
	Delay m_delayRight_6;
	Delay m_delayDetectRight_4;
	int __exception;
	int vectorsize;
	t_sample m_l_DETECT_24;
	t_sample m_smoothedGain_21;
	t_sample m_k_DELTA_25;
	t_sample m_m_AUTOREL_23;
	t_sample m_n_LOOKAHEAD_22;
	t_sample m_j_TRIM_26;
	t_sample m_e_REL_30;
	t_sample m_h_BYPASS_28;
	t_sample m_a_GAIN_31;
	t_sample m_i_MAKEUP_27;
	t_sample m_smoothedCeiling_20;
	t_sample m_g_DITHER_29;
	t_sample m_smoothedBypass_19;
	t_sample m_lookaheadHistory_15;
	t_sample m_envelopeFollower_17;
	t_sample m_prevDetection_9;
	t_sample m_transientDetector_10;
	t_sample m_rmsSum_8;
	t_sample samplerate;
	t_sample m_smoothedMakeup_18;
	t_sample m_autoReleaseHistory_11;
	t_sample m_deltaHistory_13;
	t_sample m_gainReduction_16;
	t_sample m_detectHistory_12;
	t_sample m_b_CELLING_32;
	t_sample m_trimHistory_14;
	t_sample m_d_ATK_33;
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
		m_rmsSum_8 = ((int)0);
		m_prevDetection_9 = ((int)0);
		m_transientDetector_10 = ((int)0);
		m_autoReleaseHistory_11 = ((int)0);
		m_detectHistory_12 = ((int)0);
		m_deltaHistory_13 = ((int)0);
		m_trimHistory_14 = ((int)0);
		m_lookaheadHistory_15 = ((int)0);
		m_gainReduction_16 = ((int)0);
		m_envelopeFollower_17 = ((int)0);
		m_smoothedMakeup_18 = ((int)0);
		m_smoothedBypass_19 = ((int)0);
		m_smoothedCeiling_20 = ((int)0);
		m_smoothedGain_21 = ((int)0);
		m_n_LOOKAHEAD_22 = 0;
		m_m_AUTOREL_23 = 0;
		m_l_DETECT_24 = 0;
		m_k_DELTA_25 = 0;
		m_j_TRIM_26 = 0;
		m_i_MAKEUP_27 = 0;
		m_h_BYPASS_28 = 0;
		m_g_DITHER_29 = 0;
		m_e_REL_30 = 200;
		m_a_GAIN_31 = 0;
		m_b_CELLING_32 = -0.3;
		m_d_ATK_33 = 100;
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
		t_sample ceilingLinear = dbtoa(m_b_CELLING_32);
		t_sample maxb_34 = floor(((((int)3) * samplerate) * ((t_sample)0.001)));
		t_sample rmsWindowSize = ((((int)1) < maxb_34) ? maxb_34 : ((int)1));
		t_sample rmsWindowInv = safediv(((int)1), rmsWindowSize);
		t_sample attackTime = ((m_d_ATK_33 * ((t_sample)0.001)) * samplerate);
		t_sample attackCoeff = exp(safediv(((t_sample)-0.99967234081321), attackTime));
		t_sample ceilingNegative = (ceilingLinear * ((int)-1));
		t_sample ceilingPositive = (ceilingLinear * ((int)1));
		t_sample choice_38 = int(m_g_DITHER_29);
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			const t_sample in2 = (*(__in2++));
			t_sample smoothedTrim = ((m_trimHistory_14 * ((t_sample)0.999)) + (m_j_TRIM_26 * ((t_sample)0.001)));
			m_trimHistory_14 = fixdenorm(smoothedTrim);
			t_sample trimLinear = dbtoa(smoothedTrim);
			m_smoothedGain_21 = ((m_smoothedGain_21 * ((t_sample)0.999)) + ((-m_a_GAIN_31) * ((t_sample)0.001)));
			m_smoothedGain_21 = fixdenorm(m_smoothedGain_21);
			t_sample thresholdLinear = safediv(((int)1), safepow(((int)10), (m_smoothedGain_21 * ((t_sample)0.05))));
			m_smoothedCeiling_20 = ((m_smoothedCeiling_20 * ((t_sample)0.999)) + (m_b_CELLING_32 * ((t_sample)0.001)));
			m_smoothedCeiling_20 = fixdenorm(m_smoothedCeiling_20);
			m_smoothedBypass_19 = ((m_smoothedBypass_19 * ((t_sample)0.999)) + (m_h_BYPASS_28 * ((t_sample)0.001)));
			m_smoothedBypass_19 = fixdenorm(m_smoothedBypass_19);
			t_sample wetAmount = (((int)1) - m_smoothedBypass_19);
			m_smoothedMakeup_18 = ((m_smoothedMakeup_18 * ((t_sample)0.999)) + (m_i_MAKEUP_27 * ((t_sample)0.001)));
			m_smoothedMakeup_18 = fixdenorm(m_smoothedMakeup_18);
			t_sample makeupLinear = dbtoa(m_smoothedMakeup_18);
			t_sample smoothedDelta = ((m_deltaHistory_13 * ((t_sample)0.999)) + (m_k_DELTA_25 * ((t_sample)0.001)));
			m_deltaHistory_13 = fixdenorm(smoothedDelta);
			t_sample smoothedDetect = ((m_detectHistory_12 * ((t_sample)0.999)) + (m_l_DETECT_24 * ((t_sample)0.001)));
			m_detectHistory_12 = fixdenorm(smoothedDetect);
			t_sample smoothedAutoRelease = ((m_autoReleaseHistory_11 * ((t_sample)0.999)) + (m_m_AUTOREL_23 * ((t_sample)0.001)));
			m_autoReleaseHistory_11 = fixdenorm(smoothedAutoRelease);
			t_sample leftTrimmed = (in1 * trimLinear);
			t_sample rightTrimmed = (in2 * trimLinear);
			t_sample rightScaled = ((ceilingLinear * rightTrimmed) * thresholdLinear);
			t_sample leftScaled = ((ceilingLinear * leftTrimmed) * thresholdLinear);
			t_sample averageSignal = ((leftScaled + rightScaled) * ((t_sample)0.5));
			t_sample averageAbs = fabs(averageSignal);
			t_sample peakDetection = averageAbs;
			t_sample signalSquared = (averageSignal * averageSignal);
			t_sample oldestSquared = m_rmsDelay_1.read_step(rmsWindowSize);
			t_sample rmsSumNew = ((signalSquared + m_rmsSum_8) - oldestSquared);
			t_sample rmsSumClipped = ((((int)0) < rmsSumNew) ? rmsSumNew : ((int)0));
			t_sample rmsDetection = sqrt((rmsSumClipped * rmsWindowInv));
			m_rmsSum_8 = rmsSumClipped;
			m_rmsDelay_1.write(signalSquared);
			t_sample mix_1003 = (peakDetection + (smoothedDetect * (rmsDetection - peakDetection)));
			t_sample detectionSignal = mix_1003;
			t_sample finalReleaseTime = m_e_REL_30;
			if ((smoothedAutoRelease > ((t_sample)0.01))) {
				t_sample signalChange = fabs((detectionSignal - m_prevDetection_9));
				m_prevDetection_9 = detectionSignal;
				t_sample maxb_35 = (detectionSignal * ((t_sample)0.1));
				t_sample relativeThreshold = ((((t_sample)0.001) < maxb_35) ? maxb_35 : ((t_sample)0.001));
				int cond_36 = (signalChange > relativeThreshold);
				int isTransient = (cond_36 ? ((int)1) : ((int)0));
				m_transientDetector_10 = ((m_transientDetector_10 * ((t_sample)0.99)) + (isTransient * ((t_sample)0.01)));
				t_sample mix_1004 = (((int)150) + (m_transientDetector_10 * ((int)-145)));
				t_sample autoRelease = mix_1004;
				t_sample mix_1005 = (m_e_REL_30 + (smoothedAutoRelease * (autoRelease - m_e_REL_30)));
				finalReleaseTime = mix_1005;
				
			};
			t_sample releaseTime = ((finalReleaseTime * ((t_sample)0.001)) * samplerate);
			t_sample releaseCoeff = exp(safediv(((t_sample)-0.99967234081321), releaseTime));
			t_sample maxb_37 = (m_envelopeFollower_17 * releaseCoeff);
			m_envelopeFollower_17 = ((detectionSignal < maxb_37) ? maxb_37 : detectionSignal);
			m_gainReduction_16 = ((m_gainReduction_16 * attackCoeff) + (m_envelopeFollower_17 * (((int)1) - attackCoeff)));
			m_gainReduction_16 = ((m_gainReduction_16 < ((t_sample)1e-06)) ? ((t_sample)1e-06) : m_gainReduction_16);
			m_gainReduction_16 = fixdenorm(m_gainReduction_16);
			t_sample gainReductionDb = atodb(m_gainReduction_16);
			m_envelopeFollower_17 = fixdenorm(m_envelopeFollower_17);
			t_sample limitedDb = softkneeLimiter_d_d_i(gainReductionDb, m_smoothedCeiling_20, ((int)0));
			t_sample smoothedLookahead = ((m_lookaheadHistory_15 * ((t_sample)0.99)) + (m_n_LOOKAHEAD_22 * ((t_sample)0.01)));
			m_lookaheadHistory_15 = fixdenorm(smoothedLookahead);
			int lookaheadSamples = int(((smoothedLookahead * ((t_sample)0.001)) * samplerate));
			t_sample delayedDetectLeft = m_delayDetectLeft_5.read_step(lookaheadSamples);
			t_sample delayedDetectRight = m_delayDetectRight_4.read_step(lookaheadSamples);
			t_sample delayedLeftOriginal = m_leftInputDelay_3.read_step(lookaheadSamples);
			t_sample delayedRightOriginal = m_rightInputDelay_2.read_step(lookaheadSamples);
			t_sample noise1 = ((noise() * ((t_sample)1.5258789062e-05)) * ((t_sample)0.5));
			t_sample noise2 = ((noise() * ((t_sample)1.5258789062e-05)) * ((t_sample)0.5));
			t_sample ditherTPDF = (noise1 + noise2);
			t_sample ditherGated = (((choice_38 >= 1) && (choice_38 < 2)) ? ditherTPDF : 0);
			t_sample gainReductionAmount = (limitedDb - gainReductionDb);
			t_sample gainReductionAmount_973 = ((gainReductionAmount < ((int)-144)) ? ((int)-144) : gainReductionAmount);
			t_sample gainReductionLinear = dbtoa(gainReductionAmount_973);
			t_sample totalGainApplied = (trimLinear * thresholdLinear);
			t_sample leftProcessed = (delayedDetectLeft * gainReductionLinear);
			t_sample leftClipped = ((leftProcessed <= ceilingNegative) ? ceilingNegative : ((leftProcessed >= ceilingPositive) ? ceilingPositive : leftProcessed));
			t_sample leftWithMakeup = (leftClipped * makeupLinear);
			t_sample sub_1007 = (((int)1) - smoothedDelta);
			t_sample mix_1006 = (leftClipped + (sub_1007 * (leftWithMakeup - leftClipped)));
			t_sample leftDelta = safediv((delayedDetectLeft - leftClipped), (ceilingLinear * ((totalGainApplied < ((int)1)) ? ((int)1) : totalGainApplied)));
			t_sample mix_1008 = (mix_1006 + (smoothedDelta * (leftDelta - mix_1006)));
			t_sample mix_1009 = (delayedLeftOriginal + (wetAmount * (mix_1008 - delayedLeftOriginal)));
			t_sample expr_980 = (mix_1009 + ditherGated);
			t_sample rightProcessed = (delayedDetectRight * gainReductionLinear);
			t_sample rightClipped = ((rightProcessed <= ceilingNegative) ? ceilingNegative : ((rightProcessed >= ceilingPositive) ? ceilingPositive : rightProcessed));
			t_sample rightWithMakeup = (rightClipped * makeupLinear);
			t_sample sub_1011 = (((int)1) - smoothedDelta);
			t_sample mix_1010 = (rightClipped + (sub_1011 * (rightWithMakeup - rightClipped)));
			t_sample rightDelta = safediv((delayedDetectRight - rightClipped), (ceilingLinear * ((totalGainApplied < ((int)1)) ? ((int)1) : totalGainApplied)));
			t_sample mix_1012 = (mix_1010 + (smoothedDelta * (rightDelta - mix_1010)));
			t_sample mix_1013 = (delayedRightOriginal + (wetAmount * (mix_1012 - delayedRightOriginal)));
			t_sample expr_981 = (mix_1013 + ditherGated);
			m_delayDetectRight_4.write(rightScaled);
			m_delayDetectLeft_5.write(leftScaled);
			m_delayRight_6.write(rightTrimmed);
			m_delayLeft_7.write(leftTrimmed);
			m_rightInputDelay_2.write(in2);
			m_leftInputDelay_3.write(in1);
			t_sample mix_1014 = (((int)1) + (wetAmount * (gainReductionLinear - ((int)1))));
			t_sample gainReductionOutput = ((mix_1014 <= ((int)0)) ? ((int)0) : ((mix_1014 >= ((int)1)) ? ((int)1) : mix_1014));
			t_sample out3 = gainReductionOutput;
			t_sample out2 = expr_981;
			t_sample out1 = expr_980;
			t_sample out4 = leftTrimmed;
			t_sample out5 = rightTrimmed;
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
		m_n_LOOKAHEAD_22 = (_value < 0 ? 0 : (_value > 5 ? 5 : _value));
	};
	inline void set_m_AUTOREL(t_param _value) {
		m_m_AUTOREL_23 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_l_DETECT(t_param _value) {
		m_l_DETECT_24 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_k_DELTA(t_param _value) {
		m_k_DELTA_25 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_j_TRIM(t_param _value) {
		m_j_TRIM_26 = (_value < -12 ? -12 : (_value > 12 ? 12 : _value));
	};
	inline void set_i_MAKEUP(t_param _value) {
		m_i_MAKEUP_27 = (_value < -12 ? -12 : (_value > 12 ? 12 : _value));
	};
	inline void set_h_BYPASS(t_param _value) {
		m_h_BYPASS_28 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_g_DITHER(t_param _value) {
		m_g_DITHER_29 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_e_REL(t_param _value) {
		m_e_REL_30 = (_value < 1 ? 1 : (_value > 1000 ? 1000 : _value));
	};
	inline void set_a_GAIN(t_param _value) {
		m_a_GAIN_31 = (_value < 0 ? 0 : (_value > 24 ? 24 : _value));
	};
	inline void set_b_CELLING(t_param _value) {
		m_b_CELLING_32 = (_value < -60 ? -60 : (_value > 0 ? 0 : _value));
	};
	inline void set_d_ATK(t_param _value) {
		m_d_ATK_33 = (_value < 0.01 ? 0.01 : (_value > 750 ? 750 : _value));
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
int num_params() { return 12; }

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
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		case 0: *value = self->m_a_GAIN_31; break;
		case 1: *value = self->m_b_CELLING_32; break;
		case 2: *value = self->m_d_ATK_33; break;
		case 3: *value = self->m_e_REL_30; break;
		case 4: *value = self->m_g_DITHER_29; break;
		case 5: *value = self->m_h_BYPASS_28; break;
		case 6: *value = self->m_i_MAKEUP_27; break;
		case 7: *value = self->m_j_TRIM_26; break;
		case 8: *value = self->m_k_DELTA_25; break;
		case 9: *value = self->m_l_DETECT_24; break;
		case 10: *value = self->m_m_AUTOREL_23; break;
		case 11: *value = self->m_n_LOOKAHEAD_22; break;
		
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
	self->__commonstate.params = (ParamInfo *)genlib_sysmem_newptr(12 * sizeof(ParamInfo));
	self->__commonstate.numparams = 12;
	// initialize parameter 0 ("m_a_GAIN_31")
	pi = self->__commonstate.params + 0;
	pi->name = "a_GAIN";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_a_GAIN_31;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 24;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 1 ("m_b_CELLING_32")
	pi = self->__commonstate.params + 1;
	pi->name = "b_CELLING";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_b_CELLING_32;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -60;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 2 ("m_d_ATK_33")
	pi = self->__commonstate.params + 2;
	pi->name = "d_ATK";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_d_ATK_33;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.01;
	pi->outputmax = 750;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 3 ("m_e_REL_30")
	pi = self->__commonstate.params + 3;
	pi->name = "e_REL";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_e_REL_30;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 1;
	pi->outputmax = 1000;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 4 ("m_g_DITHER_29")
	pi = self->__commonstate.params + 4;
	pi->name = "g_DITHER";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_g_DITHER_29;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_h_BYPASS_28")
	pi = self->__commonstate.params + 5;
	pi->name = "h_BYPASS";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_h_BYPASS_28;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 6 ("m_i_MAKEUP_27")
	pi = self->__commonstate.params + 6;
	pi->name = "i_MAKEUP";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_i_MAKEUP_27;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -12;
	pi->outputmax = 12;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_j_TRIM_26")
	pi = self->__commonstate.params + 7;
	pi->name = "j_TRIM";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_j_TRIM_26;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -12;
	pi->outputmax = 12;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 8 ("m_k_DELTA_25")
	pi = self->__commonstate.params + 8;
	pi->name = "k_DELTA";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_k_DELTA_25;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 9 ("m_l_DETECT_24")
	pi = self->__commonstate.params + 9;
	pi->name = "l_DETECT";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_l_DETECT_24;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 10 ("m_m_AUTOREL_23")
	pi = self->__commonstate.params + 10;
	pi->name = "m_AUTOREL";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_m_AUTOREL_23;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 11 ("m_n_LOOKAHEAD_22")
	pi = self->__commonstate.params + 11;
	pi->name = "n_LOOKAHEAD";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_n_LOOKAHEAD_22;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 5;
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
