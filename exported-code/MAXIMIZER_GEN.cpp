#include "MAXIMIZER_GEN.h"

namespace MAXIMIZER_GEN {

/*******************************************************************************************************************
Cycling '74 License for Max-Generated Code for Export
Copyright (c) 2016 Cycling '74
The code that Max generates automatically and that end users are capable of exporting and using, and any
  associated documentation files (the “Software”) is a work of authorship for which Cycling '74 is the author
  and owner for copyright purposes.  A license is hereby granted, free of charge, to any person obtaining a
  copy of the Software (“Licensee”) to use, copy, modify, merge, publish, and distribute copies of the Software,
  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The Software is licensed to Licensee only for non-commercial use. Users who wish to make commercial use of the
  Software must contact the copyright owner to determine if a license for commercial use is available, and the
  terms and conditions for same, which may include fees or royalties. For commercial use, please send inquiries
  to licensing (at) cycling74.com.  The determination of whether a use is commercial use or non-commercial use is based
  upon the use, not the user. The Software may be used by individuals, institutions, governments, corporations, or
  other business whether for-profit or non-profit so long as the use itself is not a commercialization of the
  materials or a use that generates or is intended to generate income, revenue, sales or profit.
The above copyright notice and this license shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
  THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*******************************************************************************************************************/

// global noise generator
Noise noise;
static const int GENLIB_LOOPCOUNT_BAIL = 100000;


// The State struct contains all the state and procedures for the gendsp kernel
typedef struct State {
	CommonState __commonstate;
	Change __m_change_19;
	Delay m_delay_1;
	Delay m_delay_4;
	Delay m_delay_3;
	Delay m_delay_2;
	int __exception;
	int vectorsize;
	t_sample m_e_REL_15;
	t_sample m_f_REL_14;
	t_sample m_b_CELLING_13;
	t_sample m_g_DITHER_16;
	t_sample m_d_ATK_18;
	t_sample m_i_UNITY_17;
	t_sample m_a_THD_12;
	t_sample m_history_9;
	t_sample m_history_10;
	t_sample m_history_5;
	t_sample samplerate;
	t_sample m_h_BYPASS_11;
	t_sample m_history_6;
	t_sample m_history_8;
	t_sample m_history_7;
	t_sample __m_latch_20;
	t_sample __m_slide_21;
	// re-initialize all member variables;
	inline void reset(t_param __sr, int __vs) {
		__exception = 0;
		vectorsize = __vs;
		samplerate = __sr;
		m_delay_1.reset("m_delay_1", samplerate);
		m_delay_2.reset("m_delay_2", samplerate);
		m_delay_3.reset("m_delay_3", samplerate);
		m_delay_4.reset("m_delay_4", samplerate);
		m_history_5 = ((int)0);
		m_history_6 = ((int)0);
		m_history_7 = ((int)0);
		m_history_8 = ((int)0);
		m_history_9 = ((int)0);
		m_history_10 = ((int)0);
		m_h_BYPASS_11 = 0;
		m_a_THD_12 = 0;
		m_b_CELLING_13 = 0;
		m_f_REL_14 = 50;
		m_e_REL_15 = 80;
		m_g_DITHER_16 = 0;
		m_i_UNITY_17 = 0;
		m_d_ATK_18 = 1;
		__m_change_19.reset(0);
		__m_latch_20 = 0;
		__m_slide_21 = 0;
		genlib_reset_complete(this);
		
	};
	// the signal processing routine;
	inline int perform(t_sample ** __ins, t_sample ** __outs, int __n) {
		vectorsize = __n;
		const t_sample * __in1 = __ins[0];
		const t_sample * __in2 = __ins[1];
		t_sample * __out1 = __outs[0];
		t_sample * __out2 = __outs[1];
		if (__exception) {
			return __exception;
			
		} else if (( (__in1 == 0) || (__in2 == 0) || (__out1 == 0) || (__out2 == 0) )) {
			__exception = GENLIB_ERR_NULL_BUFFER;
			return __exception;
			
		};
		t_sample mul_21110 = (m_b_CELLING_13 * ((t_sample)0.001));
		t_sample mul_21253 = (m_a_THD_12 * ((t_sample)0.001));
		t_sample add_21099 = (m_e_REL_15 + m_f_REL_14);
		t_sample mul_21439 = (m_i_UNITY_17 * ((t_sample)0.001));
		t_sample mul_21090 = (m_h_BYPASS_11 * ((t_sample)0.001));
		t_sample dbtoa_21083 = dbtoa(m_b_CELLING_13);
		t_sample mul_21166 = (m_d_ATK_18 * ((t_sample)0.001));
		t_sample mul_21167 = (mul_21166 * samplerate);
		t_sample div_21168 = safediv(((t_sample)-0.99967234081321), mul_21167);
		t_sample exp_21169 = exp(div_21168);
		t_sample gen_21171 = exp_21169;
		t_sample rsub_21152 = (((int)1) - gen_21171);
		t_sample iup_22 = (1 / maximum(1, abs(((int)50))));
		t_sample idown_23 = (1 / maximum(1, abs(((int)10))));
		t_sample mul_21159 = (add_21099 * ((t_sample)0.001));
		t_sample mul_21160 = (mul_21159 * samplerate);
		t_sample div_21161 = safediv(((t_sample)-0.99967234081321), mul_21160);
		t_sample exp_21162 = exp(div_21161);
		t_sample gen_21164 = exp_21162;
		t_sample mul_21080 = (dbtoa_21083 * (-1));
		t_sample mul_21079 = (dbtoa_21083 * ((int)1));
		t_sample mul_21097 = (((t_sample)0.002) * samplerate);
		int int_21096 = int(mul_21097);
		t_sample choice_24 = int(m_g_DITHER_16);
		int maxb_25 = (-144);
		int maxb_27 = (-144);
		// the main sample loop;
		while ((__n--)) {
			const t_sample in1 = (*(__in1++));
			const t_sample in2 = (*(__in2++));
			t_sample mul_21109 = (m_history_10 * ((t_sample)0.999));
			t_sample add_21111 = (mul_21110 + mul_21109);
			t_sample gen_21113 = add_21111;
			t_sample history_21108_next_21112 = fixdenorm(add_21111);
			t_sample mul_21252 = (m_history_9 * ((t_sample)0.999));
			t_sample add_21254 = (mul_21253 + mul_21252);
			t_sample gen_21256 = add_21254;
			t_sample history_21251_next_21255 = fixdenorm(add_21254);
			t_sample expr_22365 = safediv(((int)1), safepow(((int)10), (gen_21256 * ((t_sample)0.05))));
			t_sample mul_21438 = (m_history_8 * ((t_sample)0.999));
			t_sample add_21440 = (mul_21439 + mul_21438);
			t_sample gen_21442 = add_21440;
			t_sample history_21437_next_21441 = fixdenorm(add_21440);
			t_sample mul_21089 = (m_history_7 * ((t_sample)0.999));
			t_sample add_21091 = (mul_21090 + mul_21089);
			t_sample gen_21093 = add_21091;
			t_sample history_21088_next_21092 = fixdenorm(add_21091);
			t_sample rsub_21094 = (((int)1) - gen_21093);
			t_sample mul_21077 = (dbtoa_21083 * in2);
			t_sample mul_21084 = (mul_21077 * expr_22365);
			t_sample mul_21078 = (in1 * dbtoa_21083);
			t_sample mul_21086 = (mul_21078 * expr_22365);
			t_sample mul_21154 = (m_history_5 * gen_21171);
			t_sample mul_21147 = ((mul_21086 + mul_21084) * ((t_sample)0.5));
			t_sample abs_21145 = fabs(mul_21147);
			int gt_21143 = (mul_21147 > ((int)0));
			int change_21142 = __m_change_19(gt_21143);
			__m_latch_20 = ((change_21142 != 0) ? ((int)1) : __m_latch_20);
			int latch_21141 = __m_latch_20;
			__m_slide_21 = fixdenorm((__m_slide_21 + (((latch_21141 > __m_slide_21) ? iup_22 : idown_23) * (latch_21141 - __m_slide_21))));
			t_sample slide_21140 = __m_slide_21;
			t_sample mul_21144 = (abs_21145 * slide_21140);
			t_sample gen_21146 = mul_21144;
			t_sample mul_21155 = (m_history_6 * gen_21164);
			t_sample max_21157 = ((gen_21146 < mul_21155) ? mul_21155 : gen_21146);
			t_sample mul_21153 = (max_21157 * rsub_21152);
			t_sample add_21151 = (mul_21153 + mul_21154);
			t_sample max_21149 = ((add_21151 < ((t_sample)1e-06)) ? ((t_sample)1e-06) : add_21151);
			t_sample atodb_21148 = atodb(max_21149);
			t_sample history_21156_next_22359 = fixdenorm(max_21157);
			t_sample history_21150_next_22360 = fixdenorm(add_21151);
			t_sample expr_22363 = softkneeLimiter_d_d_i(atodb_21148, gen_21113, ((int)0));
			t_sample tap_21107 = m_delay_4.read_step(int_21096);
			t_sample tap_21103 = m_delay_3.read_step(int_21096);
			t_sample tap_21105 = m_delay_2.read_step(int_21096);
			t_sample dbtoa_21476 = dbtoa(gen_21256);
			t_sample tap_21101 = m_delay_1.read_step(int_21096);
			t_sample dbtoa_21392 = dbtoa(gen_21256);
			t_sample noise_21076 = noise();
			t_sample mul_21075 = (noise_21076 * ((t_sample)1.5258789062e-05));
			t_sample gate_21074 = ((choice_24 >= 1) ? mul_21075 : 0);
			t_sample sub_21120 = (expr_22363 - atodb_21148);
			t_sample max_21095 = ((sub_21120 < maxb_25) ? maxb_25 : sub_21120);
			t_sample dbtoa_21119 = dbtoa(max_21095);
			t_sample mul_21121 = (tap_21105 * dbtoa_21119);
			t_sample v_26 = (mul_21121 + gate_21074);
			t_sample clamp_21082 = ((v_26 <= mul_21080) ? mul_21080 : ((v_26 >= mul_21079) ? mul_21079 : v_26));
			t_sample mul_21478 = (clamp_21082 * dbtoa_21476);
			t_sample mix_22377 = (clamp_21082 + (gen_21442 * (mul_21478 - clamp_21082)));
			t_sample mix_22378 = (tap_21107 + (rsub_21094 * (mix_22377 - tap_21107)));
			t_sample out1 = mix_22378;
			t_sample sub_21116 = (expr_22363 - atodb_21148);
			t_sample max_21087 = ((sub_21116 < maxb_27) ? maxb_27 : sub_21116);
			t_sample dbtoa_21115 = dbtoa(max_21087);
			t_sample mul_21117 = (tap_21101 * dbtoa_21115);
			t_sample v_28 = (gate_21074 + mul_21117);
			t_sample clamp_21081 = ((v_28 <= mul_21080) ? mul_21080 : ((v_28 >= mul_21079) ? mul_21079 : v_28));
			t_sample mul_21320 = (clamp_21081 * dbtoa_21392);
			t_sample mix_22379 = (clamp_21081 + (gen_21442 * (mul_21320 - clamp_21081)));
			t_sample mix_22380 = (tap_21103 + (rsub_21094 * (mix_22379 - tap_21103)));
			t_sample out2 = mix_22380;
			m_history_10 = history_21108_next_21112;
			m_history_9 = history_21251_next_21255;
			m_history_8 = history_21437_next_21441;
			m_history_7 = history_21088_next_21092;
			m_history_6 = history_21156_next_22359;
			m_history_5 = history_21150_next_22360;
			m_delay_4.write(in1);
			m_delay_3.write(in2);
			m_delay_2.write(mul_21086);
			m_delay_1.write(mul_21084);
			m_delay_1.step();
			m_delay_2.step();
			m_delay_3.step();
			m_delay_4.step();
			// assign results to output buffer;
			(*(__out1++)) = out1;
			(*(__out2++)) = out2;
			
		};
		return __exception;
		
	};
	inline void set_h_BYPASS(t_param _value) {
		m_h_BYPASS_11 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_a_THD(t_param _value) {
		m_a_THD_12 = (_value < -20 ? -20 : (_value > 0 ? 0 : _value));
	};
	inline void set_b_CELLING(t_param _value) {
		m_b_CELLING_13 = (_value < -60 ? -60 : (_value > 0 ? 0 : _value));
	};
	inline void set_f_REL2(t_param _value) {
		m_f_REL_14 = (_value < 5 ? 5 : (_value > 500 ? 500 : _value));
	};
	inline void set_e_REL(t_param _value) {
		m_e_REL_15 = (_value < 1 ? 1 : (_value > 1500 ? 1500 : _value));
	};
	inline void set_g_DITHER(t_param _value) {
		m_g_DITHER_16 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_i_UNITY(t_param _value) {
		m_i_UNITY_17 = (_value < 0 ? 0 : (_value > 1 ? 1 : _value));
	};
	inline void set_d_ATK(t_param _value) {
		m_d_ATK_18 = (_value < 0.01 ? 0.01 : (_value > 500 ? 500 : _value));
	};
	inline t_sample softkneeLimiter_d_d_i(t_sample xg, t_sample T, int W) {
		t_sample _softkneeLimiter_ret_22362 = ((int)0);
		if (((((int)2) * (xg - T)) < (W * (-((int)1))))) {
			_softkneeLimiter_ret_22362 = xg;
			
		} else {
			if (((((int)2) * fabs((xg - T))) <= W)) {
				_softkneeLimiter_ret_22362 = (xg - safediv(safepow(((xg - T) - (W * ((t_sample)0.5))), ((int)2)), (((int)2) * W)));
				
			} else {
				if (((((int)2) * (xg - T)) > W)) {
					_softkneeLimiter_ret_22362 = T;
					
				};
				
			};
			
		};
		return _softkneeLimiter_ret_22362;
		
	};
	
} State;


///
///	Configuration for the genlib API
///

/// Number of signal inputs and outputs

int gen_kernel_numins = 2;
int gen_kernel_numouts = 2;

int num_inputs() { return gen_kernel_numins; }
int num_outputs() { return gen_kernel_numouts; }
int num_params() { return 8; }

/// Assistive lables for the signal inputs and outputs

const char *gen_kernel_innames[] = { "in1", "in2" };
const char *gen_kernel_outnames[] = { "out1", "out2" };

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
		case 0: self->set_a_THD(value); break;
		case 1: self->set_b_CELLING(value); break;
		case 2: self->set_d_ATK(value); break;
		case 3: self->set_e_REL(value); break;
		case 4: self->set_f_REL2(value); break;
		case 5: self->set_g_DITHER(value); break;
		case 6: self->set_h_BYPASS(value); break;
		case 7: self->set_i_UNITY(value); break;
		
		default: break;
	}
}

/// Get the value of a parameter of a State object

void getparameter(CommonState *cself, long index, t_param *value) {
	State *self = (State *)cself;
	switch (index) {
		case 0: *value = self->m_a_THD_12; break;
		case 1: *value = self->m_b_CELLING_13; break;
		case 2: *value = self->m_d_ATK_18; break;
		case 3: *value = self->m_e_REL_15; break;
		case 4: *value = self->m_f_REL_14; break;
		case 5: *value = self->m_g_DITHER_16; break;
		case 6: *value = self->m_h_BYPASS_11; break;
		case 7: *value = self->m_i_UNITY_17; break;
		
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
	self->__commonstate.params = (ParamInfo *)genlib_sysmem_newptr(8 * sizeof(ParamInfo));
	self->__commonstate.numparams = 8;
	// initialize parameter 0 ("m_a_THD_12")
	pi = self->__commonstate.params + 0;
	pi->name = "a_THD";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_a_THD_12;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -20;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 1 ("m_b_CELLING_13")
	pi = self->__commonstate.params + 1;
	pi->name = "b_CELLING";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_b_CELLING_13;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = -60;
	pi->outputmax = 0;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 2 ("m_d_ATK_18")
	pi = self->__commonstate.params + 2;
	pi->name = "d_ATK";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_d_ATK_18;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0.01;
	pi->outputmax = 500;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 3 ("m_e_REL_15")
	pi = self->__commonstate.params + 3;
	pi->name = "e_REL";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_e_REL_15;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 1;
	pi->outputmax = 1500;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 4 ("m_f_REL_14")
	pi = self->__commonstate.params + 4;
	pi->name = "f_REL2";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_f_REL_14;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 5;
	pi->outputmax = 500;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 5 ("m_g_DITHER_16")
	pi = self->__commonstate.params + 5;
	pi->name = "g_DITHER";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_g_DITHER_16;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 6 ("m_h_BYPASS_11")
	pi = self->__commonstate.params + 6;
	pi->name = "h_BYPASS";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_h_BYPASS_11;
	pi->defaultref = 0;
	pi->hasinputminmax = false;
	pi->inputmin = 0;
	pi->inputmax = 1;
	pi->hasminmax = true;
	pi->outputmin = 0;
	pi->outputmax = 1;
	pi->exp = 0;
	pi->units = "";		// no units defined
	// initialize parameter 7 ("m_i_UNITY_17")
	pi = self->__commonstate.params + 7;
	pi->name = "i_UNITY";
	pi->paramtype = GENLIB_PARAMTYPE_FLOAT;
	pi->defaultvalue = self->m_i_UNITY_17;
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


} // MAXIMIZER_GEN::
