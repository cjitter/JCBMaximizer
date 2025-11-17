{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 9,
			"minor" : 0,
			"revision" : 8,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 164.0, 165.0, 1614.0, 733.0 ],
		"gridsize" : [ 15.0, 15.0 ],
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-34",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1454.0, 425.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-22",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1454.0, 460.0, 79.0, 22.0 ],
					"presentation_linecount" : 2,
					"text" : "o_DCFILT $1"
				}

			}
, 			{
				"box" : 				{
					"attr" : "autoexport",
					"id" : "obj-26",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 410.0, 87.0, 138.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-25",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 483.0, 30.0, 301.0, 20.0 ],
					"text" : "/Users/jcb/JUCEProjects/JCBMaximizer/exported-code"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-37",
					"maxclass" : "newobj",
					"numinlets" : 0,
					"numoutlets" : 0,
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 9,
							"minor" : 0,
							"revision" : 8,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "box",
						"rect" : [ 175.0, 193.0, 1572.0, 666.0 ],
						"gridsize" : [ 15.0, 15.0 ],
						"boxes" : [ 							{
								"box" : 								{
									"fontsize" : 35.0,
									"id" : "obj-2",
									"linecount" : 15,
									"maxclass" : "comment",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 23.0, 23.0, 1460.0, 632.0 ],
									"text" : "// ====================================\n// DECLARACIÓN DE PARÁMETROS\n// ====================================\nParam d_ATK(0, min=0.01, default=100, max=750);    // Tiempo de ataque en ms\nParam b_CELLING(0, min=-60, default=-0.3, max=0);  // Techo máximo en dB\nParam a_GAIN(0, min=0, default=0, max=24);         // Input gain (dB) - moderno\nParam e_REL(0, min=1, default=200, max=1000);      // Tiempo de release en ms\nParam g_DITHER(0, min=0, default=0, max=1);        // Cantidad de dither (0-1)\nParam h_BYPASS(0, min=0, default=0, max=1);        // Bypass del efecto (0-1)\nParam i_MAKEUP(0, min=-12, default=0, max=12);     // Makeup gain post-proceso (dB)\nParam j_TRIM(-12, min=-12, default=0, max=12);     // Input trim gain (dB)\nParam k_DELTA(0, min=0, default=0, max=1);         // Delta mode (0-1)\nParam l_DETECT(0, min=0, default=0, max=1);        // Detection mode (0=Peak, 1=RMS)\nParam m_AUTOREL(0, min=0, default=0, max=1);       // Auto-release enable (0-1)\nParam n_LOOKAHEAD(0, min=0, default=0, max=5);     // Lookahead time (ms)\n"
								}

							}
 ],
						"lines" : [  ]
					}
,
					"patching_rect" : [ 468.0, 157.0, 68.0, 22.0 ],
					"text" : "p PARAMS"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-35",
					"maxclass" : "flonum",
					"maximum" : 12.0,
					"minimum" : -12.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1380.0, 265.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-31",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1380.0, 305.0, 85.0, 22.0 ],
					"text" : "i_MAKEUP $1"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-10",
					"maxclass" : "flonum",
					"maximum" : 24.0,
					"minimum" : 0.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 662.0, 242.0, 50.0, 22.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ -10.0 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "number[4]",
							"parameter_mmax" : 24.0,
							"parameter_modmode" : 0,
							"parameter_shortname" : "number[6]",
							"parameter_type" : 0
						}

					}
,
					"varname" : "number[1]"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-18",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 662.0, 291.0, 68.0, 22.0 ],
					"text" : "a_GAIN $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-54",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 474.0, 482.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-53",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 381.0, 482.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"bgcolor" : [ 1.0, 1.0, 1.0, 1.0 ],
					"id" : "obj-52",
					"maxclass" : "multislider",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 321.0, 486.0, 24.0, 78.0 ],
					"setminmax" : [ 0.0, 1.0 ],
					"setstyle" : 1,
					"slidercolor" : [ 1.0, 0.0, 0.070588235294118, 0.99 ]
				}

			}
, 			{
				"box" : 				{
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-51",
					"interval" : 20.0,
					"maxclass" : "number~",
					"mode" : 2,
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "float" ],
					"patching_rect" : [ 284.0, 457.0, 56.0, 22.0 ],
					"sig" : 0.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-45",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1111.0, 124.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-43",
					"maxclass" : "flonum",
					"maximum" : 1.0,
					"minimum" : 0.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1111.0, 155.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-39",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1111.0, 182.0, 98.0, 22.0 ],
					"text" : "m_AUTOREL $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-38",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 978.0, 204.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-36",
					"maxclass" : "flonum",
					"maximum" : 1.0,
					"minimum" : 0.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 978.0, 243.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-30",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 978.0, 291.0, 82.0, 22.0 ],
					"text" : "l_DETECT $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-29",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1417.0, 353.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-27",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1417.0, 389.0, 74.0, 22.0 ],
					"text" : "k_DELTA $1"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-19",
					"maxclass" : "flonum",
					"maximum" : 5.0,
					"minimum" : 0.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 854.0, 243.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-15",
					"maxclass" : "flonum",
					"maximum" : 12.0,
					"minimum" : -12.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 590.0, 243.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-7",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 854.0, 291.0, 113.0, 22.0 ],
					"text" : "n_LOOKAHEAD $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-2",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 590.0, 291.0, 64.0, 22.0 ],
					"text" : "j_TRIM $1"
				}

			}
, 			{
				"box" : 				{
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-50",
					"maxclass" : "number~",
					"mode" : 2,
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "float" ],
					"patching_rect" : [ 223.0, 641.0, 80.0, 22.0 ],
					"sig" : 0.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-49",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1319.0, 118.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-47",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1319.0, 157.0, 83.0, 22.0 ],
					"text" : "g_DITHER $1"
				}

			}
, 			{
				"box" : 				{
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-33",
					"maxclass" : "number~",
					"mode" : 2,
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "float" ],
					"patching_rect" : [ 117.0, 641.0, 80.0, 22.0 ],
					"sig" : 0.0
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-32",
					"maxclass" : "flonum",
					"maximum" : 0.0,
					"minimum" : -20.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 735.0, 242.0, 51.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-20",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 205.0, 587.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-6",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 101.0, 587.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-105",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1348.0, 189.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-103",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1348.0, 222.0, 86.0, 22.0 ],
					"text" : "h_BYPASS $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-8",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 121.0, 101.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-3",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 121.0, 131.0, 65.0, 22.0 ],
					"text" : "modout $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-4",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 492.0, 434.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"bgcolor" : [ 0.094117647058824, 0.501960784313725, 0.082352941176471, 1.0 ],
					"bgcolor2" : [ 0.094117647058824, 0.501960784313725, 0.082352941176471, 1.0 ],
					"bgfillcolor_angle" : 270.0,
					"bgfillcolor_autogradient" : 0.0,
					"bgfillcolor_color" : [ 0.066666666666667, 0.407843137254902, 0.054901960784314, 1.0 ],
					"bgfillcolor_color1" : [ 0.094117647058824, 0.501960784313725, 0.082352941176471, 1.0 ],
					"bgfillcolor_color2" : [ 0.2, 0.2, 0.2, 1.0 ],
					"bgfillcolor_proportion" : 0.5,
					"bgfillcolor_type" : "color",
					"gradient" : 1,
					"id" : "obj-23",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 436.0, 120.0, 68.0, 22.0 ],
					"text" : "exportcode"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-21",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 64.0, 59.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-17",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 84.0, 430.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-14",
					"maxclass" : "newobj",
					"numinlets" : 0,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 132.0, 339.0, 61.0, 22.0 ],
					"text" : "r toMaxim"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-13",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 919.0, 625.0, 63.0, 22.0 ],
					"text" : "s toMaxim"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-16",
					"maxclass" : "flonum",
					"maximum" : 750.0,
					"minimum" : 0.001,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 1069.0, 242.0, 57.0, 22.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ 15.0 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "number[3]",
							"parameter_mmax" : 750.0,
							"parameter_mmin" : 0.001,
							"parameter_modmode" : 0,
							"parameter_shortname" : "number[3]",
							"parameter_type" : 0
						}

					}
,
					"varname" : "number[3]"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-24",
					"maxclass" : "flonum",
					"maximum" : 1000.0,
					"minimum" : 1.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 1144.0, 242.0, 50.0, 22.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ 80.0 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "number[2]",
							"parameter_mmax" : 1000.0,
							"parameter_mmin" : 1.0,
							"parameter_modmode" : 0,
							"parameter_shortname" : "number[2]",
							"parameter_type" : 0
						}

					}
,
					"varname" : "number[2]"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-40",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1144.0, 291.0, 61.0, 22.0 ],
					"text" : "e_REL $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-41",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1069.0, 291.0, 61.0, 22.0 ],
					"text" : "d_ATK $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-62",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 735.0, 291.0, 90.0, 22.0 ],
					"text" : "b_CELLING $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-12",
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 141.0, 544.0, 45.0, 45.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-11",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 98.0, 59.0, 88.0, 22.0 ],
					"text" : "open, loop 1, 1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-9",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 3,
					"outlettype" : [ "signal", "signal", "bang" ],
					"patching_rect" : [ 98.0, 171.0, 226.0, 22.0 ],
					"text" : "sfplay~ 2"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 5,
					"outlettype" : [ "signal", "signal", "signal", "signal", "signal" ],
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 9,
							"minor" : 0,
							"revision" : 8,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "dsp.gen",
						"rect" : [ 758.0, 87.0, 1102.0, 903.0 ],
						"gridsize" : [ 15.0, 15.0 ],
						"boxes" : [ 							{
								"box" : 								{
									"id" : "obj-8",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 1017.0, 834.0, 35.0, 22.0 ],
									"text" : "out 5"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-7",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 776.5, 834.0, 35.0, 22.0 ],
									"text" : "out 4"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-6",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 536.0, 834.0, 35.0, 22.0 ],
									"text" : "out 3"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-5",
									"maxclass" : "newobj",
									"numinlets" : 0,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 1017.0, 29.0, 28.0, 22.0 ],
									"text" : "in 2"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-3",
									"maxclass" : "newobj",
									"numinlets" : 0,
									"numoutlets" : 1,
									"outlettype" : [ "" ],
									"patching_rect" : [ 55.0, 25.0, 28.0, 22.0 ],
									"text" : "in 1"
								}

							}
, 							{
								"box" : 								{
									"code" : "// ====================================\n// FUNCIÓN SOFT KNEE LIMITER\n// ====================================\n// Aplica limitación con transición suave (soft knee)\n// xg: nivel de entrada en dB\n// threshold: umbral de limitación en dB\n// kneeWidth: ancho de la transición suave en dB\nsoftkneeLimiter(xg, threshold, kneeWidth) {\n    ret = 0;\n    \n    // Si la señal está muy por debajo del umbral (fuera de la rodilla)\n    if ((2 * (xg - threshold)) < (kneeWidth * (-1))) {\n        ret = xg;  // No hay reducción\n    }\n    // Si la señal está dentro de la zona de transición suave (rodilla)\n    else if ((2 * abs(xg - threshold)) <= kneeWidth) {\n        // Aplica reducción cuadrática para transición suave\n        ret = xg - (pow((xg - threshold) - (kneeWidth / 2), 2) / (2 * kneeWidth));\n    }\n    // Si la señal está por encima del umbral (fuera de la rodilla)\n    else if ((2 * (xg - threshold)) > kneeWidth) {\n        ret = threshold;  // Limitación total al umbral\n    }\n    \n    return ret;\n}\n\n\n// ====================================\n// DECLARACIÓN DE PARÁMETROS\n// ====================================\nParam d_ATK(0, min=0.01, default=100, max=750);    // Tiempo de ataque en ms\nParam b_CELLING(0, min=-60, default=-0.3, max=0);  // Techo máximo en dB\nParam a_GAIN(0, min=0, default=0, max=24);         // Input gain (dB) - moderno\nParam e_REL(0, min=1, default=200, max=1000);      // Tiempo de release en ms\nParam g_DITHER(0, min=0, default=0, max=1);        // Cantidad de dither (0-1)\nParam h_BYPASS(0, min=0, default=0, max=1);        // Bypass del efecto (0-1)\nParam i_MAKEUP(0, min=-12, default=0, max=12);     // Makeup gain post-proceso (dB)\nParam j_TRIM(-12, min=-12, default=0, max=12);     // Input trim gain (dB)\nParam k_DELTA(0, min=0, default=0, max=1);         // Delta mode (0-1)\nParam l_DETECT(0, min=0, default=0, max=1);        // Detection mode (0=Peak, 1=RMS)\nParam m_AUTOREL(0, min=0, default=0, max=1);       // Auto-release enable (0-1)\n\nParam o_DCFILT(0, min=0, default=0, max=1);   // Filtro DC post (0=off,1=on)\nParam n_LOOKAHEAD(0, min=0, default=0, max=5);     // Lookahead time (ms)\n\n// ====================================\n// VARIABLES DE ESTADO (HISTORIALES)\n// ====================================\nHistory smoothedGain(0);         // Suavizado del gain (antes threshold)\nHistory smoothedCeiling(0);      // Suavizado del techo\nHistory smoothedBypass(0);       // Suavizado del bypass\nHistory smoothedMakeup(0);       // Suavizado del makeup gain\nHistory envelopeFollower(0);     // Seguidor de envolvente para detección RMS\nHistory gainReduction(0);        // Reducción de ganancia actual\nHistory lookaheadHistory(0);     // Para suavizado del lookahead\nHistory trimHistory(0);          // Para suavizado del trim\nHistory deltaHistory(0);         // Para suavizado del modo delta\nHistory detectHistory(0);        // Para suavizado del modo de detección\nHistory autoReleaseHistory(0);   // Para suavizado del auto-release\nHistory transientDetector(0);    // Detector de transientes para auto-release\nHistory prevDetection(0);        // Detección anterior para calcular cambios\nHistory rmsSum(0);               // Suma para RMS sliding window\n\nHistory progRmsStateL(0);        // RMS corto del programa L\nHistory progRmsStateR(0);        // RMS corto del programa R\nHistory deltaRmsStateL(0);       // RMS corto del delta L\nHistory deltaRmsStateR(0);       // RMS corto del delta R\n\nHistory hpfPrevInL(0);\nHistory hpfPrevInR(0);\nHistory hpfStateL(0);\nHistory hpfStateR(0);\nHistory dcFiltHistory(0);        // Suavizado del conmutador o_DCFILT (anti-click)\n\n// ====================================\n// LÍNEAS DE DELAY PARA LOOKAHEAD\n// ====================================\nDelay delayLeft(samplerate);        // Delay canal izquierdo (con trim)\nDelay delayRight(samplerate);       // Delay canal derecho (con trim)\nDelay delayDetectLeft(samplerate);  // Delay detección izquierda\nDelay delayDetectRight(samplerate); // Delay detección derecha\nDelay leftInputDelay(samplerate);   // Delay canal izquierdo (sin trim para bypass)\nDelay rightInputDelay(samplerate);  // Delay canal derecho (sin trim para bypass)\nDelay rmsDelay(500);                // Buffer para RMS sliding window (500 samples ~10ms @ 48kHz)\n\n// ====================================\n// CONSTANTES GLOBALES\n// ====================================\nditherAmount = 1.5258789062e-05;       // Cantidad de ruido para dither\nSMOOTH_HISTORY_FACTOR = 0.999;         // Factor de suavizado para valores históricos (99.9%)\nSMOOTH_PARAM_FACTOR = 0.001;           // Factor de suavizado para nuevos parámetros (0.1%)\nLOOKAHEAD_SMOOTH_FACTOR = 0.99;        // Factor de suavizado para lookahead\nexpConstant = -0.99967234081321;       // Constante para cálculo exponencial de tiempos\n\n// ====================================\n// SUAVIZADO DE PARÁMETROS\n// ====================================\n// Suavizado del trim\nsmoothedTrim = trimHistory * 0.999 + j_TRIM * 0.001;\ntrimHistory = fixdenorm(smoothedTrim);\ntrimLinear = dbtoa(smoothedTrim);\n\n// Suavizado del threshold con factor 0.999\nsmoothedGain = smoothedGain * 0.999 + (-a_GAIN) * 0.001;\nsmoothedGain = fixdenorm(smoothedGain);\n\n// Conversión de threshold a factor lineal\nthresholdLinear = 1 / pow(10, smoothedGain / 20);\n\n// Suavizado del ceiling\nsmoothedCeiling = smoothedCeiling * 0.999 + b_CELLING * 0.001;\nsmoothedCeiling = fixdenorm(smoothedCeiling);\n\n// Suavizado del bypass\nsmoothedBypass = smoothedBypass * 0.999 + h_BYPASS * 0.001;\nsmoothedBypass = fixdenorm(smoothedBypass);\nwetAmount = 1 - smoothedBypass;  // Cantidad de señal procesada\n\n// Suavizado del makeup gain\nsmoothedMakeup = smoothedMakeup * 0.999 + i_MAKEUP * 0.001;\nsmoothedMakeup = fixdenorm(smoothedMakeup);\nmakeupLinear = dbtoa(smoothedMakeup);\n\n// Suavizado del modo delta\nsmoothedDelta = deltaHistory * 0.999 + k_DELTA * 0.001;\ndeltaHistory = fixdenorm(smoothedDelta);\n\n// Suavizado del modo de detección\nsmoothedDetect = detectHistory * 0.999 + l_DETECT * 0.001;\ndetectHistory = fixdenorm(smoothedDetect);\n\n// Suavizado del auto-release\nsmoothedAutoRelease = autoReleaseHistory * 0.999 + m_AUTOREL * 0.001;\nautoReleaseHistory = fixdenorm(smoothedAutoRelease);\n\n// Suavizado del filtro DC (anti-click al conmutar)\nsmoothedDCFILT = dcFiltHistory * 0.95 + o_DCFILT * 0.05;  // ataque/relajación rápidos\ndcFiltHistory = fixdenorm(smoothedDCFILT);\n\n// ====================================\n// PREPARACIÓN DE SEÑALES DE ENTRADA\n// ====================================\n// Aplicar trim a las entradas\nleftTrimmed = in1 * trimLinear;\nrightTrimmed = in2 * trimLinear;\n\n// Conversión de ceiling a lineal\nceilingLinear = dbtoa(b_CELLING);\n\n// Aplicar ceiling y threshold a las entradas CON TRIM\nrightScaled = ceilingLinear * rightTrimmed * thresholdLinear;\nleftScaled = ceilingLinear * leftTrimmed * thresholdLinear;\n\n// PRE-WRITE de todos los delays usados luego en read(...)\ndelayDetectLeft.write(leftScaled);\ndelayDetectRight.write(rightScaled);\ndelayLeft.write(leftTrimmed);          // con trim (rama WET)\ndelayRight.write(rightTrimmed);        // con trim (rama WET)\nleftInputDelay.write(in1);             // sin trim (bypass real)\nrightInputDelay.write(in2);            // sin trim (bypass real)\n\n// ====================================\n// DETECCIÓN DE NIVEL\n// ====================================\n// Promedio de ambos canales para detección\naverageSignal = (leftScaled + rightScaled) * 0.5;\n\n// DC-blocker nativo gen~\ndetIn = dcblock(averageSignal);\n\n// Peak detection\npeakDetection = abs(detIn);\n\n// RMS sliding window (ventana de ~3ms)\nrmsWindowSize = max(1, floor(3 * samplerate * 0.001));  // 3ms en samples\nrmsWindowInv = 1 / rmsWindowSize;\n\n// RMS sliding window con detIn\nsignalSquared = detIn * detIn;\n\n// PRE-WRITE\nrmsDelay.write(signalSquared);\n\n// Lo correcto con pre-write es leer N (no N-1)\noldestSquared = rmsDelay.read(rmsWindowSize, interp=\"none\");\n\nrmsSumNew = (signalSquared + rmsSum) - oldestSquared;\nrmsSumClipped = max(0, rmsSumNew);\nrmsDetection = sqrt(rmsSumClipped * rmsWindowInv);\n\n// Historial\nrmsSum = fixdenorm(rmsSumClipped);\n\n// Interpolar entre Peak y RMS según el parámetro\ndetectionSignal = mix(peakDetection, rmsDetection, smoothedDetect);\n\n// ====================================\n// CÁLCULO DE CONSTANTES DE TIEMPO Y AUTO-RELEASE\n// ====================================\n\n// Auto-release: detectar transientes y ajustar release dinámicamente\nfinalReleaseTime = e_REL;  // Por defecto usar release manual\n\nif(smoothedAutoRelease > 0.01) {\n    // Calcular cambio en la señal\n    signalChange = abs(detectionSignal - prevDetection);\n    prevDetection = detectionSignal;\n    \n    // Detectar si es transiente (cambio rápido) o sostenido\n    relativeThreshold = max(0.001, detectionSignal * 0.1);  // 10% del nivel actual\n    isTransient = signalChange > relativeThreshold ? 1 : 0;\n    \n    // Suavizar detección de transientes\n    transientSmooth = 0.99;  // Respuesta rápida\n    transientDetector = (transientDetector * transientSmooth) + (isTransient * (1 - transientSmooth));\n    \n    // Definir rangos de release para limitador\n    fastRelease = 5;     // 5ms para transientes (muy rápido)\n    slowRelease = 150;   // 150ms para material sostenido\n    \n    // Interpolar basado en contenido detectado\n    autoRelease = mix(slowRelease, fastRelease, transientDetector);\n    \n    // Mezclar release manual con auto\n    finalReleaseTime = mix(e_REL, autoRelease, smoothedAutoRelease);\n}\n\n// Constante de tiempo para release final\nreleaseTime = finalReleaseTime * 0.001 * samplerate;\nreleaseCoeff = exp(expConstant / releaseTime);\n\n// Seguidor de envolvente con release\nenvelopeFollower = max(detectionSignal, envelopeFollower * releaseCoeff);\n\n// Constante de tiempo para attack\nattackTime = d_ATK * 0.001 * samplerate;\nattackCoeff = exp(expConstant / attackTime);\n\n// Aplicar attack al seguidor de envolvente\ngainReduction = gainReduction * attackCoeff + envelopeFollower * (1 - attackCoeff);\ngainReduction = max(gainReduction, 0.000001);  // Evitar valores negativos o cero\ngainReduction = fixdenorm(gainReduction);\n\n// Conversión a dB para procesar con soft knee\ngainReductionDb = atodb(gainReduction);\n\n// Actualizar historiales para siguiente muestra\nenvelopeFollower = fixdenorm(envelopeFollower);\n\n// ====================================\n// APLICACIÓN DE SOFT KNEE LIMITER\n// ====================================\n// Hard knee fijo (knee = 0) para evitar discontinuidades\n\nlimitedDb = softkneeLimiter(gainReductionDb, smoothedCeiling, 0);\n\n// ====================================\n// CÁLCULO DE REDUCCIÓN DE GANANCIA (adelantado para usarlo en brickwall)\n// ====================================\n// Diferencia entre señal limitada y original (en dB)\ngainReductionAmount = limitedDb - gainReductionDb;\ngainReductionAmount = max(gainReductionAmount, -144);  // Limitar a -144dB mínimo\ngainReductionLinear = dbtoa(gainReductionAmount);\n\n// Calcular ganancia total aplicada para normalización del delta\n// (Puede no usarse si el delta se normaliza respecto a programDriven*)\ntotalGainApplied = trimLinear * thresholdLinear;\n\n// ====================================\n// PREPARACIÓN DE LÍMITES Y LOOKAHEAD\n// ====================================\nceilingNegative = ceilingLinear * -1;  // Límite negativo\nceilingPositive = ceilingLinear * 1;   // Límite positivo\n\n// Suavizado del lookahead (más rápido pero aún suave para evitar clicks)\nsmoothedLookahead = lookaheadHistory * 0.99 + n_LOOKAHEAD * 0.01;\nlookaheadHistory = fixdenorm(smoothedLookahead);\n\n// Convertir tiempo de lookahead (ms) a muestras\nlookaheadSamples = round(mstosamps(smoothedLookahead));\n\n// Por seguridad, clamp del índice (tu lookahead máx 5 ms << samplerate)\n//lh = max(0, lookaheadSamples);\nlh = clamp(lookaheadSamples, 0, samplerate - 1);\n\n// Lecturas en STEP para evitar interpolación y mantener PDC limpia\ndelayedLeft          = delayLeft.read(lh, interp=\"step\");\ndelayedRight         = delayRight.read(lh, interp=\"step\");\ndelayedDetectLeft    = delayDetectLeft.read(lh, interp=\"step\");\ndelayedDetectRight   = delayDetectRight.read(lh, interp=\"step\");\ndelayedLeftOriginal  = leftInputDelay.read(lh, interp=\"step\");\ndelayedRightOriginal = rightInputDelay.read(lh, interp=\"step\");\n\n// ====================================\n// BRICKWALL: GANANCIA INSTANTÁNEA POR MUESTRA (lookahead)\n// ====================================\n// Ruta de PROGRAMA: usar la señal post-trim y post-lookahead\nprogramL = delayedLeft;\nprogramR = delayedRight;\n\n// Incluir el \"drive\" del maximizador (a_GAIN) mediante thresholdLinear\nprogramDrivenL = programL * thresholdLinear;\nprogramDrivenR = programR * thresholdLinear;\n\n// Cálculo de ganancia segura por canal para encajar en ceiling sobre la señal *con drive*\n// g_br_* <= 1 siempre; evita división por cero con epsilon\nbw_eps = 1e-12;\ng_br_L = min(1, ceilingLinear / max(abs(programDrivenL), bw_eps));\ng_br_R = min(1, ceilingLinear / max(abs(programDrivenR), bw_eps));\n\n// Conservar la envolvente temporal (attack/release + auto-release) como compresión musical\n// sin perder la seguridad brickwall: usar el mínimo de ambos factores\nfinalGain_L = min(g_br_L, gainReductionLinear);\nfinalGain_R = min(g_br_R, gainReductionLinear);\n\n// ====================================\n// GANANCIA DE COMPENSACIÓN (UNITY/MAKEUP)\n// ====================================\n// Nota: Esta línea ya no se usa, makeupLinear se calcula arriba\n// makeupGainLinear = dbtoa(smoothedThreshold);\n\n// ====================================\n// GENERACIÓN DE DITHER (TPDF) DESCORRELACIONADO L/R\n// ====================================\n// gen::noise() es bipolar; usamos dos pares independientes por canal\n// para TPDF descorrelacionado entre L y R\nnoiseL1 = noise() * ditherAmount * 0.5;\nnoiseL2 = noise() * ditherAmount * 0.5;\nditherTPDF_L = noiseL1 + noiseL2;\n\nnoiseR1 = noise() * ditherAmount * 0.5;\nnoiseR2 = noise() * ditherAmount * 0.5;\nditherTPDF_R = noiseR1 + noiseR2;\n\nditherGatedL = gate(g_DITHER, ditherTPDF_L);\nditherGatedR = gate(g_DITHER, ditherTPDF_R);\n\n// (Cálculo de ganancia de reducción movido más arriba, justo tras softkneeLimiter)\n\n// ====================================\n// PROCESAMIENTO CANAL IZQUIERDO\n// ====================================\n// Usar ganancia brickwall y drive sobre la ruta de programa\nleftProcessed = programDrivenL * finalGain_L;\n\n// HPF 1º ORDEN PRE-CEILING (ON/OFF)\n// (mover desde post-makeup a pre-clamp para que el ceiling controle picos del HPF)\n// Coeficiente común (12 Hz)\ndc_fc = 12;\ndc_r  = exp(-2 * pi * dc_fc / samplerate);\n\n// y = x - x_prev + r*y_prev (canal L)\nhpfOutL_pre = (leftProcessed - hpfPrevInL) + dc_r * hpfStateL;\n// Actualizar estados L\nhpfPrevInL = leftProcessed;\nhpfStateL  = hpfOutL_pre;\n\n// Selección ON/OFF antes del clamp\nleftPre = mix(leftProcessed, hpfOutL_pre, smoothedDCFILT);\n\n// Aplicar clipping (SIN dither aquí)\nleftClipped = clamp(leftPre, ceilingNegative, ceilingPositive);\nleftDeltaRaw = programDrivenL - leftClipped;   // delta crudo (antes de makeup)\n\n// Aplicar makeup gain SOLO cuando NO está en modo delta\nleftWithMakeup = leftClipped * makeupLinear;\nleftFinalProcessed = mix(leftClipped, leftWithMakeup, 1 - smoothedDelta);\n\n// (Se asignará después de calcular leftDeltaNorm/rightDeltaNorm)\nleftWithDelta = leftFinalProcessed;\n\n\n// ====================================\n// PROCESAMIENTO CANAL DERECHO\n// ====================================\n// Usar ganancia brickwall y drive sobre la ruta de programa\nrightProcessed = programDrivenR * finalGain_R;\n\n// HPF 1º ORDEN PRE-CEILING (ON/OFF) - canal R\nhpfOutR_pre = (rightProcessed - hpfPrevInR) + dc_r * hpfStateR;\n// Actualizar estados R\nhpfPrevInR = rightProcessed;\nhpfStateR  = hpfOutR_pre;\n\n// Selección ON/OFF antes del clamp\nrightPre = mix(rightProcessed, hpfOutR_pre, smoothedDCFILT);\n\n// Aplicar clipping (SIN dither aquí)\nrightClipped = clamp(rightPre, ceilingNegative, ceilingPositive);\nrightDeltaRaw = programDrivenR - rightClipped; // delta crudo (antes de makeup)\n\n// ====================================\n// DELTA AUTO-NORMALIZADO (RMS corto ~20ms) – POR CANAL\n// ====================================\nrmsTimeSamples = max(1, floor(0.02 * samplerate));     // 20 ms\nrmsCoeff = exp(expConstant / rmsTimeSamples);\n\n// RMS del programa por canal\nprogLSq = programDrivenL * programDrivenL;\nprogRSq = programDrivenR * programDrivenR;\nprogRmsStateL = progRmsStateL * rmsCoeff + progLSq * (1 - rmsCoeff);\nprogRmsStateR = progRmsStateR * rmsCoeff + progRSq * (1 - rmsCoeff);\nprogRmsL = sqrt(max(progRmsStateL, 1e-12));\nprogRmsR = sqrt(max(progRmsStateR, 1e-12));\n\n// RMS del delta por canal\ndeltaLSq = leftDeltaRaw * leftDeltaRaw;\ndeltaRSq = rightDeltaRaw * rightDeltaRaw;\ndeltaRmsStateL = deltaRmsStateL * rmsCoeff + deltaLSq * (1 - rmsCoeff);\ndeltaRmsStateR = deltaRmsStateR * rmsCoeff + deltaRSq * (1 - rmsCoeff);\ndeltaRmsL = sqrt(max(deltaRmsStateL, 1e-12));\ndeltaRmsR = sqrt(max(deltaRmsStateR, 1e-12));\n\n// Normalizar delta al RMS del programa por canal (SOLO HACIA ABAJO: no hacemos boost)\nmaxDeltaBoost = 1; // no se usa para subir; mantenemos por claridad\nnormGainL = min(1, progRmsL / max(deltaRmsL, 1e-12));\nnormGainR = min(1, progRmsR / max(deltaRmsR, 1e-12));\n\n\nleftDeltaNorm  = leftDeltaRaw  * normGainL;\nrightDeltaNorm = rightDeltaRaw * normGainR;\n\n// ====================================\n// DELTA LOUDNESS GUARD (dependiente de GR)\n// ====================================\n// Atenúa el delta cuando la reducción es muy grande para evitar crujidos\n// Ganancia efectiva media (finalGain ya incluye brickwall ∧ envolvente)\ngrLinAvg = (finalGain_L + finalGain_R) * 0.5;\ngrDb = atodb(max(grLinAvg, 1e-12)); // dB <= 0 cuando hay reducción\n\n// Empezar a atenuar delta por encima de 6 dB de GR y llegar a máximo a 18 dB\n// t = 0  -> sin atenuación; t = 1 -> atenuación máxima\n// Factor final: 1.0 -> 0.35 (≈ -9 dB)\nt = clamp((-grDb - 6) / 12, 0, 1);\ndeltaLoudnessFactor = mix(1.0, 0.35, t);\n\nleftDeltaNorm  = leftDeltaNorm  * deltaLoudnessFactor;\nrightDeltaNorm = rightDeltaNorm * deltaLoudnessFactor;\n// ====================================\n// DELTA PEAK SAFETY (evita overs/clip del delta)\n// ====================================\ndeltaPeakGuardL = min(1, ceilingLinear / max(abs(leftDeltaNorm), 1e-12));\ndeltaPeakGuardR = min(1, ceilingLinear / max(abs(rightDeltaNorm), 1e-12));\nleftDeltaSafe  = leftDeltaNorm  * deltaPeakGuardL;\nrightDeltaSafe = rightDeltaNorm * deltaPeakGuardR;\n\n// Aplicar makeup gain SOLO cuando NO está en modo delta\nrightWithMakeup = rightClipped * makeupLinear;\nrightFinalProcessed = mix(rightClipped, rightWithMakeup, 1 - smoothedDelta);\n\n// (Se asignará después de calcular leftDeltaNorm/rightDeltaNorm)\nrightWithDelta = rightFinalProcessed;\n\n// Aplicar delta auto-normalizado (tras el cálculo común)\nleftWithDelta  = mix(leftFinalProcessed,  leftDeltaSafe,  smoothedDelta);\nrightWithDelta = mix(rightFinalProcessed, rightDeltaSafe, smoothedDelta);\n\n// Mezclar con señal original según bypass (tras calcular deltas normalizados)\nleftFinal = mix(delayedLeftOriginal, leftWithDelta, wetAmount);\n\nrightFinal = mix(delayedRightOriginal, rightWithDelta, wetAmount);\n\n// Aplicar dither como último paso (FASE INVERTIDA PARA PRUEBA)\nout2 = rightFinal + ditherGatedR;\nout1 = leftFinal  + ditherGatedL;\n\n// ====================================\n// GAIN REDUCTION OUTPUT - Medidor de reducción para UI\n// ====================================\n// Mostrar la ganancia efectiva (mínimo entre brickwall y envolvente)\nfinalGainDisplay = (min(g_br_L, gainReductionLinear) + min(g_br_R, gainReductionLinear)) * 0.5;\ngainReductionMeter = mix(1, finalGainDisplay, wetAmount);\ngainReductionOutput = clamp(gainReductionMeter, 0, 1);\n\n// ====================================\n// SALIDAS\n// ====================================\n// out1, out2 ya definidas arriba (audio procesado L/R)\nout3 = gainReductionOutput;        // Gain reduction para medidor (0-1) LINEAL\nout4 = leftTrimmed;               // Señal post-trim L (para medidores de entrada)\nout5 = rightTrimmed;              // Señal post-trim R (para medidores de entrada)",
									"fontface" : 0,
									"fontname" : "<Monospaced>",
									"fontsize" : 12.0,
									"id" : "obj-2",
									"maxclass" : "codebox",
									"numinlets" : 2,
									"numoutlets" : 5,
									"outlettype" : [ "", "", "", "", "" ],
									"patching_rect" : [ 55.0, 68.0, 981.0, 740.0 ]
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-31",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 295.5, 834.0, 35.0, 22.0 ],
									"text" : "out 2"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-4",
									"maxclass" : "newobj",
									"numinlets" : 1,
									"numoutlets" : 0,
									"patching_rect" : [ 55.0, 834.0, 35.0, 22.0 ],
									"text" : "out 1"
								}

							}
 ],
						"lines" : [ 							{
								"patchline" : 								{
									"destination" : [ "obj-31", 0 ],
									"source" : [ "obj-2", 1 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-4", 0 ],
									"source" : [ "obj-2", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-6", 0 ],
									"source" : [ "obj-2", 2 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-7", 0 ],
									"source" : [ "obj-2", 3 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-8", 0 ],
									"source" : [ "obj-2", 4 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-2", 0 ],
									"source" : [ "obj-3", 0 ]
								}

							}
, 							{
								"patchline" : 								{
									"destination" : [ "obj-2", 1 ],
									"source" : [ "obj-5", 0 ]
								}

							}
 ],
						"styles" : [ 							{
								"name" : "AudioStatus_Menu",
								"default" : 								{
									"bgfillcolor" : 									{
										"angle" : 270.0,
										"autogradient" : 0,
										"color" : [ 0.294118, 0.313726, 0.337255, 1 ],
										"color1" : [ 0.454902, 0.462745, 0.482353, 0.0 ],
										"color2" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
										"proportion" : 0.39,
										"type" : "color"
									}

								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "Audiomix",
								"default" : 								{
									"bgfillcolor" : 									{
										"angle" : 270.0,
										"color" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
										"color1" : [ 0.376471, 0.384314, 0.4, 1.0 ],
										"color2" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
										"proportion" : 0.39,
										"type" : "gradient"
									}

								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "Max 12 Regular",
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "jx.test1",
								"default" : 								{
									"bgfillcolor" : 									{
										"angle" : 270.0,
										"color" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
										"color1" : [ 0.376471, 0.384314, 0.4, 1.0 ],
										"color2" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
										"proportion" : 0.39,
										"type" : "gradient"
									}
,
									"fontface" : [ 1 ],
									"fontname" : [ "Verdana" ],
									"fontsize" : [ 9.0 ],
									"textcolor" : [ 0.32549, 0.345098, 0.372549, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "ksliderWhite",
								"default" : 								{
									"color" : [ 1.0, 1.0, 1.0, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "newobjBlue-1",
								"default" : 								{
									"accentcolor" : [ 0.317647, 0.654902, 0.976471, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "newobjBrown-1",
								"default" : 								{
									"accentcolor" : [ 0.654902, 0.572549, 0.376471, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "newobjCyan-1",
								"default" : 								{
									"accentcolor" : [ 0.029546, 0.773327, 0.821113, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "newobjGreen-1",
								"default" : 								{
									"accentcolor" : [ 0.0, 0.533333, 0.168627, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "newobjYellow-1",
								"default" : 								{
									"accentcolor" : [ 0.82517, 0.78181, 0.059545, 1.0 ],
									"fontsize" : [ 12.059008 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "numberGold-1",
								"default" : 								{
									"accentcolor" : [ 0.764706, 0.592157, 0.101961, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
, 							{
								"name" : "rsliderGold",
								"default" : 								{
									"bgcolor" : [ 0.764706, 0.592157, 0.101961, 1.0 ],
									"color" : [ 0.646639, 0.821777, 0.854593, 1.0 ]
								}
,
								"parentstyle" : "",
								"multi" : 0
							}
 ]
					}
,
					"patching_rect" : [ 98.0, 426.0, 392.0, 22.0 ],
					"saved_object_attributes" : 					{
						"autoexport" : 0,
						"exportfolder" : "Sequoia:/Users/jcb/JUCEProjects/JCBMaximizer/exported-code/",
						"exportname" : "JCBMaximizer"
					}
,
					"text" : "gen~ @title JCBMaximizer @autoexport 0 @exportname JCBMaximizer"
				}

			}
, 			{
				"box" : 				{
					"attr" : "exportfolder",
					"id" : "obj-5",
					"maxclass" : "attrui",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 384.0, 54.0, 560.0, 22.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 1 ],
					"midpoints" : [ 200.75, 472.0, 176.5, 472.0 ],
					"order" : 2,
					"source" : [ "obj-1", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 0 ],
					"midpoints" : [ 107.5, 472.0, 150.5, 472.0 ],
					"order" : 0,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-20", 0 ],
					"midpoints" : [ 200.75, 517.5, 211.0, 517.5 ],
					"order" : 1,
					"source" : [ "obj-1", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-33", 0 ],
					"midpoints" : [ 107.5, 544.5, 126.5, 544.5 ],
					"order" : 1,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-50", 0 ],
					"midpoints" : [ 200.75, 544.5, 232.5, 544.5 ],
					"order" : 0,
					"source" : [ "obj-1", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-51", 0 ],
					"source" : [ "obj-1", 2 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-53", 0 ],
					"source" : [ "obj-1", 3 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-54", 0 ],
					"source" : [ "obj-1", 4 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"midpoints" : [ 107.5, 517.5, 107.0, 517.5 ],
					"order" : 2,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-18", 0 ],
					"source" : [ "obj-10", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1357.5, 455.0, 928.5, 455.0 ],
					"source" : [ "obj-103", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-103", 0 ],
					"source" : [ "obj-105", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"midpoints" : [ 107.5, 137.0, 107.5, 137.0 ],
					"source" : [ "obj-11", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 141.5, 409.5, 107.5, 409.5 ],
					"source" : [ "obj-14", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 0 ],
					"source" : [ "obj-15", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-41", 0 ],
					"source" : [ "obj-16", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 671.5, 456.88671875, 928.5, 456.88671875 ],
					"source" : [ "obj-18", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-7", 0 ],
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 599.5, 456.0, 928.5, 456.0 ],
					"source" : [ "obj-2", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"midpoints" : [ 73.5, 138.0, 107.5, 138.0 ],
					"source" : [ "obj-21", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1463.5, 499.1640625, 928.5, 499.1640625 ],
					"source" : [ "obj-22", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 445.5, 409.5, 107.5, 409.5 ],
					"source" : [ "obj-23", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-40", 0 ],
					"source" : [ "obj-24", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 419.5, 412.0, 107.5, 412.0 ],
					"source" : [ "obj-26", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1426.5, 499.97265625, 1002.25, 499.97265625, 1002.25, 500.04296875, 928.5, 500.04296875 ],
					"source" : [ "obj-27", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-27", 0 ],
					"source" : [ "obj-29", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"midpoints" : [ 130.5, 173.0, 107.5, 173.0 ],
					"source" : [ "obj-3", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 987.5, 499.0, 928.5, 499.0 ],
					"source" : [ "obj-30", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1389.5, 475.0, 928.5, 475.0 ],
					"source" : [ "obj-31", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-62", 0 ],
					"source" : [ "obj-32", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-22", 0 ],
					"source" : [ "obj-34", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-31", 0 ],
					"source" : [ "obj-35", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-30", 0 ],
					"source" : [ "obj-36", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-36", 0 ],
					"source" : [ "obj-38", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1120.5, 223.0, 1211.5546875, 223.0, 1211.5546875, 277.0, 1211.6875, 277.0, 1211.6875, 499.60546875, 928.5, 499.60546875 ],
					"source" : [ "obj-39", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1153.5, 455.0, 928.5, 455.0 ],
					"source" : [ "obj-40", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1078.5, 455.0, 928.5, 455.0 ],
					"source" : [ "obj-41", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-39", 0 ],
					"source" : [ "obj-43", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-43", 0 ],
					"source" : [ "obj-45", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1328.5, 455.5, 928.5, 455.5 ],
					"source" : [ "obj-47", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-47", 0 ],
					"source" : [ "obj-49", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 393.5, 411.0, 107.5, 411.0 ],
					"source" : [ "obj-5", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-52", 0 ],
					"source" : [ "obj-51", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 744.5, 455.0, 928.5, 455.0 ],
					"source" : [ "obj-62", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 863.5, 457.49609375, 928.5, 457.49609375 ],
					"source" : [ "obj-7", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-3", 0 ],
					"source" : [ "obj-8", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 1 ],
					"midpoints" : [ 211.0, 355.5, 480.5, 355.5 ],
					"order" : 1,
					"source" : [ "obj-9", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 107.5, 355.5, 107.5, 355.5 ],
					"order" : 0,
					"source" : [ "obj-9", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-17", 0 ],
					"midpoints" : [ 107.5, 328.5, 90.0, 328.5 ],
					"order" : 1,
					"source" : [ "obj-9", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-4", 0 ],
					"midpoints" : [ 211.0, 355.5, 498.0, 355.5 ],
					"order" : 0,
					"source" : [ "obj-9", 1 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-10" : [ "number[4]", "number[6]", 0 ],
			"obj-16" : [ "number[3]", "number[3]", 0 ],
			"obj-24" : [ "number[2]", "number[2]", 0 ],
			"parameterbanks" : 			{

			}
,
			"inherited_shortname" : 1
		}
,
		"dependency_cache" : [  ],
		"autosave" : 0,
		"styles" : [ 			{
				"name" : "AudioStatus_Menu",
				"default" : 				{
					"bgfillcolor" : 					{
						"angle" : 270.0,
						"autogradient" : 0,
						"color" : [ 0.294118, 0.313726, 0.337255, 1 ],
						"color1" : [ 0.454902, 0.462745, 0.482353, 0.0 ],
						"color2" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
						"proportion" : 0.39,
						"type" : "color"
					}

				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "Audiomix",
				"default" : 				{
					"bgfillcolor" : 					{
						"angle" : 270.0,
						"color" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
						"color1" : [ 0.376471, 0.384314, 0.4, 1.0 ],
						"color2" : [ 0.290196, 0.309804, 0.301961, 1.0 ],
						"proportion" : 0.39,
						"type" : "gradient"
					}

				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "Max 12 Regular",
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "ksliderWhite",
				"default" : 				{
					"color" : [ 1.0, 1.0, 1.0, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "newobjBlue-1",
				"default" : 				{
					"accentcolor" : [ 0.317647, 0.654902, 0.976471, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "newobjBrown-1",
				"default" : 				{
					"accentcolor" : [ 0.654902, 0.572549, 0.376471, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "newobjCyan-1",
				"default" : 				{
					"accentcolor" : [ 0.029546, 0.773327, 0.821113, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "newobjGreen-1",
				"default" : 				{
					"accentcolor" : [ 0.0, 0.533333, 0.168627, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "newobjYellow-1",
				"default" : 				{
					"accentcolor" : [ 0.82517, 0.78181, 0.059545, 1.0 ],
					"fontsize" : [ 12.059008 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "numberGold-1",
				"default" : 				{
					"accentcolor" : [ 0.764706, 0.592157, 0.101961, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
, 			{
				"name" : "rsliderGold",
				"default" : 				{
					"bgcolor" : [ 0.764706, 0.592157, 0.101961, 1.0 ],
					"color" : [ 0.646639, 0.821777, 0.854593, 1.0 ]
				}
,
				"parentstyle" : "",
				"multi" : 0
			}
 ]
	}

}
