{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 9,
			"minor" : 0,
			"revision" : 7,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 126.0, 198.0, 1614.0, 733.0 ],
		"gridsize" : [ 15.0, 15.0 ],
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-54",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 439.5, 440.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-53",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 355.75, 440.0, 12.0, 76.0 ]
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
					"patching_rect" : [ 308.0, 439.0, 24.0, 78.0 ],
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
					"patching_rect" : [ 271.0, 410.0, 56.0, 22.0 ],
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
					"patching_rect" : [ 1234.0, 29.0, 24.0, 24.0 ]
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
					"patching_rect" : [ 1234.0, 60.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-39",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1234.0, 87.0, 98.0, 22.0 ],
					"presentation_linecount" : 2,
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
					"patching_rect" : [ 1080.0, 146.0, 24.0, 24.0 ]
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
					"patching_rect" : [ 1080.0, 185.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-30",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1080.0, 233.0, 82.0, 22.0 ],
					"presentation_linecount" : 2,
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
					"patching_rect" : [ 1400.5, 435.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-27",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1400.5, 471.0, 74.0, 22.0 ],
					"presentation_linecount" : 2,
					"text" : "k_DELTA $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-26",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 1444.0, 279.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-22",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1444.0, 322.0, 70.0, 22.0 ],
					"presentation_linecount" : 2,
					"text" : "i_UNITY $1"
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
					"patching_rect" : [ 956.0, 185.0, 50.0, 22.0 ]
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
					"patching_rect" : [ 601.0, 185.0, 50.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-7",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 956.0, 233.0, 113.0, 22.0 ],
					"presentation_linecount" : 2,
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
					"patching_rect" : [ 601.0, 233.0, 64.0, 22.0 ],
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
					"patching_rect" : [ 226.0, 594.0, 80.0, 22.0 ],
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
					"patching_rect" : [ 1394.0, 99.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-47",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1394.0, 148.0, 83.0, 22.0 ],
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
					"patching_rect" : [ 120.0, 594.0, 80.0, 22.0 ],
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
					"patching_rect" : [ 769.0, 184.0, 51.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-20",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 208.0, 540.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-6",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 104.0, 540.0, 12.0, 76.0 ]
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
					"patching_rect" : [ 1414.0, 184.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-103",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 1414.0, 233.0, 86.0, 22.0 ],
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
					"patching_rect" : [ 124.0, 147.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-3",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 124.0, 177.0, 65.0, 22.0 ],
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
					"patching_rect" : [ 457.0, 383.0, 12.0, 76.0 ]
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
					"patching_rect" : [ 387.0, 87.0, 68.0, 22.0 ],
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
					"patching_rect" : [ 67.0, 105.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-17",
					"maxclass" : "meter~",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 87.0, 383.0, 12.0, 76.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-14",
					"maxclass" : "newobj",
					"numinlets" : 0,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 135.0, 292.0, 61.0, 22.0 ],
					"text" : "r toMaxim"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-13",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 789.0, 456.0, 63.0, 22.0 ],
					"text" : "s toMaxim"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-16",
					"maxclass" : "flonum",
					"minimum" : 0.001,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 1171.0, 184.0, 57.0, 22.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ 15.0 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "number[3]",
							"parameter_mmax" : 1.0,
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
					"maximum" : 1500.0,
					"minimum" : 1.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 1246.0, 184.0, 50.0, 22.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ 80.0 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "number[2]",
							"parameter_mmax" : 1500.0,
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
					"patching_rect" : [ 1246.0, 233.0, 61.0, 22.0 ],
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
					"patching_rect" : [ 1171.0, 233.0, 61.0, 22.0 ],
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
					"patching_rect" : [ 769.0, 233.0, 90.0, 22.0 ],
					"text" : "b_CELLING $1"
				}

			}
, 			{
				"box" : 				{
					"format" : 6,
					"id" : "obj-70",
					"maxclass" : "flonum",
					"maximum" : 0.0,
					"minimum" : -20.0,
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "", "bang" ],
					"parameter_enable" : 1,
					"patching_rect" : [ 680.0, 184.0, 50.0, 22.0 ],
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_initial" : [ -10.0 ],
							"parameter_initial_enable" : 1,
							"parameter_longname" : "number[6]",
							"parameter_mmax" : 0.0,
							"parameter_mmin" : -20.0,
							"parameter_modmode" : 0,
							"parameter_shortname" : "number[6]",
							"parameter_type" : 0
						}

					}
,
					"varname" : "number[6]"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-77",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 680.0, 233.0, 63.0, 22.0 ],
					"text" : "a_THD $1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-12",
					"maxclass" : "ezdac~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 144.0, 497.0, 45.0, 45.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-11",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 101.0, 105.0, 88.0, 22.0 ],
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
					"patching_rect" : [ 101.0, 217.0, 226.0, 22.0 ],
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
							"revision" : 7,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "dsp.gen",
						"rect" : [ 217.0, 87.0, 1585.0, 904.0 ],
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
									"code" : "// ====================================\n// FUNCIÓN SOFT KNEE LIMITER\n// ====================================\n// Aplica limitación con transición suave (soft knee)\n// xg: nivel de entrada en dB\n// threshold: umbral de limitación en dB  \n// kneeWidth: ancho de la transición suave en dB\nsoftkneeLimiter(xg, threshold, kneeWidth) {\n    ret = 0;\n    \n    // Si la señal está muy por debajo del umbral (fuera de la rodilla)\n    if ((2 * (xg - threshold)) < (kneeWidth * (-1))) {\n        ret = xg;  // No hay reducción\n    }\n    // Si la señal está dentro de la zona de transición suave (rodilla)\n    else if ((2 * abs(xg - threshold)) <= kneeWidth) {\n        // Aplica reducción cuadrática para transición suave\n        ret = xg - (pow((xg - threshold) - (kneeWidth / 2), 2) / (2 * kneeWidth));\n    }\n    // Si la señal está por encima del umbral (fuera de la rodilla)\n    else if ((2 * (xg - threshold)) > kneeWidth) {\n        ret = threshold;  // Limitación total al umbral\n    }\n    \n    return ret;\n}\n\n// ====================================\n// DECLARACIÓN DE PARÁMETROS\n// ====================================\nParam d_ATK(0, min=0.01, default=1, max=500);      // Tiempo de ataque en ms\nParam b_CELLING(0, min=-60, default=0, max=0);     // Techo máximo en dB\nParam a_THD(0, min=-20, default=0, max=0);         // Umbral de limitación en dB\nParam e_REL(0, min=1, default=50, max=1500);       // Tiempo de release en ms\nParam g_DITHER(0, min=0, default=0, max=1);        // Cantidad de dither (0-1)\nParam h_BYPASS(0, min=0, default=0, max=1);        // Bypass del efecto (0-1)\nParam j_TRIM(-12, min=-12, default=0, max=12);     // Input trim gain (dB)\nParam k_DELTA(0, min=0, default=0, max=1);         // Delta mode (0-1)\nParam l_DETECT(0, min=0, default=0, max=1);        // Detection mode (0=Peak, 1=RMS)\nParam m_AUTOREL(0, min=0, default=0, max=1);       // Auto-release enable (0-1)\nParam n_LOOKAHEAD(0, min=0, default=2, max=5);     // Lookahead time (ms)\n\n// ====================================\n// VARIABLES DE ESTADO (HISTORIALES)\n// ====================================\nHistory smoothedThreshold(0);    // Suavizado del umbral\nHistory smoothedCeiling(0);      // Suavizado del techo\nHistory smoothedBypass(0);       // Suavizado del bypass\nHistory envelopeFollower(0);     // Seguidor de envolvente para detección RMS\nHistory gainReduction(0);        // Reducción de ganancia actual\nHistory lookaheadHistory(0);     // Para suavizado del lookahead\nHistory trimHistory(0);          // Para suavizado del trim\nHistory deltaHistory(0);         // Para suavizado del modo delta\nHistory detectHistory(0);        // Para suavizado del modo de detección\nHistory autoReleaseHistory(0);   // Para suavizado del auto-release\nHistory transientDetector(0);    // Detector de transientes para auto-release\nHistory prevDetection(0);        // Detección anterior para calcular cambios\nHistory rmsSum(0);               // Suma para RMS sliding window\n\n// ====================================\n// LÍNEAS DE DELAY PARA LOOKAHEAD\n// ====================================\nDelay delayLeft(samplerate);     // Delay canal izquierdo\nDelay delayRight(samplerate);    // Delay canal derecho  \nDelay delayDetectLeft(samplerate);  // Delay detección izquierda\nDelay delayDetectRight(samplerate); // Delay detección derecha\nDelay rmsDelay(500);                 // Buffer para RMS sliding window (500 samples ~10ms @ 48kHz)\n\n// ====================================\n// CONSTANTES GLOBALES\n// ====================================\nditherAmount = 1.5258789062e-05;       // Cantidad de ruido para dither\nSMOOTH_HISTORY_FACTOR = 0.999;         // Factor de suavizado para valores históricos (99.9%)\nSMOOTH_PARAM_FACTOR = 0.001;           // Factor de suavizado para nuevos parámetros (0.1%)\nLOOKAHEAD_SMOOTH_FACTOR = 0.99;        // Factor de suavizado para lookahead\nexpConstant = -0.99967234081321;       // Constante para cálculo exponencial de tiempos\n\n// ====================================\n// SUAVIZADO DE PARÁMETROS\n// ====================================\n// Suavizado del trim\nsmoothedTrim = trimHistory * 0.999 + j_TRIM * 0.001;\ntrimHistory = fixdenorm(smoothedTrim);\ntrimLinear = dbtoa(smoothedTrim);\n\n// Suavizado del threshold con factor 0.999\nsmoothedThreshold = smoothedThreshold * 0.999 + a_THD * 0.001;\nsmoothedThreshold = fixdenorm(smoothedThreshold);\n\n// Conversión de threshold a factor lineal\nthresholdLinear = 1 / pow(10, smoothedThreshold / 20);\n\n// Suavizado del ceiling\nsmoothedCeiling = smoothedCeiling * 0.999 + b_CELLING * 0.001;\nsmoothedCeiling = fixdenorm(smoothedCeiling);\n\n// Suavizado del bypass\nsmoothedBypass = smoothedBypass * 0.999 + h_BYPASS * 0.001;\nsmoothedBypass = fixdenorm(smoothedBypass);\nwetAmount = 1 - smoothedBypass;  // Cantidad de señal procesada\n\n// Suavizado del modo delta\nsmoothedDelta = deltaHistory * 0.999 + k_DELTA * 0.001;\ndeltaHistory = fixdenorm(smoothedDelta);\n\n// Suavizado del modo de detección\nsmoothedDetect = detectHistory * 0.999 + l_DETECT * 0.001;\ndetectHistory = fixdenorm(smoothedDetect);\n\n// Suavizado del auto-release\nsmoothedAutoRelease = autoReleaseHistory * 0.999 + m_AUTOREL * 0.001;\nautoReleaseHistory = fixdenorm(smoothedAutoRelease);\n\n// ====================================\n// PREPARACIÓN DE SEÑALES DE ENTRADA\n// ====================================\n// Aplicar trim a las entradas\nleftTrimmed = in1 * trimLinear;\nrightTrimmed = in2 * trimLinear;\n\n// Conversión de ceiling a lineal\nceilingLinear = dbtoa(b_CELLING);\n\n// Aplicar ceiling y threshold a las entradas CON TRIM\nrightScaled = ceilingLinear * rightTrimmed * thresholdLinear;\nleftScaled = ceilingLinear * leftTrimmed * thresholdLinear;\n\n// ====================================\n// DETECCIÓN DE NIVEL\n// ====================================\n// Promedio de ambos canales para detección\naverageSignal = (leftScaled + rightScaled) * 0.5;\naverageAbs = abs(averageSignal);\n\n// Peak detection (instantánea)\npeakDetection = averageAbs;\n\n// RMS sliding window (ventana de ~3ms)\nrmsWindowSize = max(1, floor(3 * samplerate * 0.001));  // 3ms en samples\nrmsWindowInv = 1 / rmsWindowSize;\n\n// Calcular RMS con ventana deslizante\nsignalSquared = averageSignal * averageSignal;\noldestSquared = rmsDelay.read(rmsWindowSize, interp=\"none\");\nrmsSumNew = (signalSquared + rmsSum) - oldestSquared;\nrmsSumClipped = max(0, rmsSumNew);  // Evitar valores negativos por errores de redondeo\nrmsDetection = sqrt(rmsSumClipped * rmsWindowInv);\n\n// Actualizar delay y suma\nrmsSum = rmsSumClipped;\nrmsDelay.write(signalSquared);\n\n// Interpolar entre Peak y RMS según el parámetro\ndetectionSignal = mix(peakDetection, rmsDetection, smoothedDetect);\n\n// ====================================\n// CÁLCULO DE CONSTANTES DE TIEMPO Y AUTO-RELEASE\n// ====================================\n\n// Auto-release: detectar transientes y ajustar release dinámicamente\nfinalReleaseTime = e_REL;  // Por defecto usar release manual\n\nif(smoothedAutoRelease > 0.01) {\n    // Calcular cambio en la señal\n    signalChange = abs(detectionSignal - prevDetection);\n    prevDetection = detectionSignal;\n    \n    // Detectar si es transiente (cambio rápido) o sostenido\n    relativeThreshold = max(0.001, detectionSignal * 0.1);  // 10% del nivel actual\n    isTransient = signalChange > relativeThreshold ? 1 : 0;\n    \n    // Suavizar detección de transientes\n    transientSmooth = 0.99;  // Respuesta rápida\n    transientDetector = (transientDetector * transientSmooth) + (isTransient * (1 - transientSmooth));\n    \n    // Definir rangos de release para limitador\n    fastRelease = 5;     // 5ms para transientes (muy rápido)\n    slowRelease = 150;   // 150ms para material sostenido\n    \n    // Interpolar basado en contenido detectado\n    autoRelease = mix(slowRelease, fastRelease, transientDetector);\n    \n    // Mezclar release manual con auto\n    finalReleaseTime = mix(e_REL, autoRelease, smoothedAutoRelease);\n}\n\n// Constante de tiempo para release final\nreleaseTime = finalReleaseTime * 0.001 * samplerate;\nreleaseCoeff = exp(expConstant / releaseTime);\n\n// Seguidor de envolvente con release\nenvelopeFollower = max(detectionSignal, envelopeFollower * releaseCoeff);\n\n// Constante de tiempo para attack\nattackTime = d_ATK * 0.001 * samplerate;\nattackCoeff = exp(expConstant / attackTime);\n\n// Aplicar attack al seguidor de envolvente\ngainReduction = gainReduction * attackCoeff + envelopeFollower * (1 - attackCoeff);\ngainReduction = max(gainReduction, 0.000001);  // Evitar valores negativos o cero\ngainReduction = fixdenorm(gainReduction);\n\n// Conversión a dB para procesar con soft knee\ngainReductionDb = atodb(gainReduction);\n\n// Actualizar historiales para siguiente muestra\nenvelopeFollower = fixdenorm(envelopeFollower);\n\n// ====================================\n// APLICACIÓN DE SOFT KNEE LIMITER\n// ====================================\n// Hard knee fijo (knee = 0) para evitar discontinuidades\nlimitedDb = softkneeLimiter(gainReductionDb, smoothedCeiling, 0);\n\n// ====================================\n// PREPARACIÓN DE LÍMITES Y DELAYS\n// ====================================\nceilingNegative = ceilingLinear * -1;  // Límite negativo\nceilingPositive = ceilingLinear * 1;   // Límite positivo\n\n// Suavizado del lookahead (más rápido pero aún suave para evitar clicks)\nsmoothedLookahead = lookaheadHistory * 0.99 + n_LOOKAHEAD * 0.01;\nlookaheadHistory = fixdenorm(smoothedLookahead);\n\n// Convertir tiempo de lookahead (ms) a muestras\nlookaheadSamples = int(smoothedLookahead * 0.001 * samplerate);\n\n// Leer señales retrasadas\ndelayedLeft = delayLeft.read(lookaheadSamples);\ndelayedRight = delayRight.read(lookaheadSamples);\ndelayedDetectLeft = delayDetectLeft.read(lookaheadSamples);\ndelayedDetectRight = delayDetectRight.read(lookaheadSamples);\n\n// ====================================\n// GANANCIA DE COMPENSACIÓN (UNITY/MAKEUP)\n// ====================================\n// Conversión de threshold a ganancia lineal para makeup\nmakeupGainLinear = dbtoa(smoothedThreshold);\n\n// ====================================\n// GENERACIÓN DE DITHER\n// ====================================\nditherNoise = noise() * ditherAmount;\nditherGated = gate(g_DITHER, ditherNoise);\n\n// ====================================\n// CÁLCULO DE REDUCCIÓN DE GANANCIA\n// ====================================\n// Diferencia entre señal limitada y original (en dB)\ngainReductionAmount = limitedDb - gainReductionDb;\ngainReductionAmount = max(gainReductionAmount, -144);  // Limitar a -144dB mínimo\ngainReductionLinear = dbtoa(gainReductionAmount);\n\n// ====================================\n// PROCESAMIENTO CANAL IZQUIERDO\n// ====================================\n// Aplicar reducción de ganancia\nleftProcessed = delayedDetectLeft * gainReductionLinear;\n\n// Aplicar clipping y dither\nleftClipped = clamp(leftProcessed + ditherGated, ceilingNegative, ceilingPositive);\n\n// Modo delta - calcular diferencia entre señal con trim y procesada\n// IMPORTANTE: Comparar señales con el mismo scaling\nleftDelta = delayedDetectLeft - leftClipped;\nleftWithDelta = mix(leftClipped, leftDelta, smoothedDelta);\n\n// Mezclar con señal original según bypass\nout1 = mix(delayedLeft, leftWithDelta, wetAmount);\n\n// ====================================\n// PROCESAMIENTO CANAL DERECHO\n// ====================================\n// Aplicar reducción de ganancia\nrightProcessed = delayedDetectRight * gainReductionLinear;\n\n// Aplicar clipping y dither\nrightClipped = clamp(ditherGated + rightProcessed, ceilingNegative, ceilingPositive);\n\n// Modo delta - calcular diferencia entre señal con trim y procesada\n// IMPORTANTE: Comparar señales con el mismo scaling\nrightDelta = delayedDetectRight - rightClipped;\nrightWithDelta = mix(rightClipped, rightDelta, smoothedDelta);\n\n// Mezclar con señal original según bypass\nout2 = mix(delayedRight, rightWithDelta, wetAmount);\n\n// ====================================\n// ESCRIBIR EN BUFFERS DE DELAY\n// ====================================\ndelayDetectRight.write(rightScaled);\ndelayDetectLeft.write(leftScaled);\ndelayRight.write(rightTrimmed);    // Escribir señal con trim\ndelayLeft.write(leftTrimmed);      // Escribir señal con trim\n\n// ====================================\n// GAIN REDUCTION OUTPUT - Medidor de reducción para UI\n// ====================================\n// gainReductionLinear ya contiene el factor de ganancia lineal\n// 1 = sin reducción, 0.5 = -6dB, 0.25 = -12dB, etc.\n// Este es exactamente el formato que necesitas para el medidor\ngainReductionMeter = mix(1, gainReductionLinear, wetAmount);  // 1 cuando bypass, valor real cuando activo\ngainReductionOutput = clamp(gainReductionMeter, 0, 1);\n\n// ====================================\n// SALIDAS\n// ====================================\n// out1, out2 ya definidas arriba (audio procesado L/R)\nout3 = gainReductionOutput;        // Gain reduction para medidor (0-1) LINEAL\nout4 = leftTrimmed;               // Señal post-trim L (para medidores de entrada)\nout5 = rightTrimmed;              // Señal post-trim R (para medidores de entrada)",
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
					"patching_rect" : [ 101.0, 379.0, 354.0, 22.0 ],
					"saved_object_attributes" : 					{
						"autoexport" : 0,
						"exportfolder" : "Sequoia:/Users/jcb/JUCEProjects/JCBMaximizer/exported-code/",
						"exportname" : "JCBMaximizer"
					}
,
					"text" : "gen~ @title MAXIM @autoexport 0 @exportname JCBMaximizer"
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
					"patching_rect" : [ 367.0, 56.0, 560.0, 22.0 ]
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 1 ],
					"midpoints" : [ 194.25, 425.0, 179.5, 425.0 ],
					"order" : 2,
					"source" : [ "obj-1", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 0 ],
					"midpoints" : [ 110.5, 425.0, 153.5, 425.0 ],
					"order" : 0,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-20", 0 ],
					"midpoints" : [ 194.25, 470.5, 214.0, 470.5 ],
					"order" : 1,
					"source" : [ "obj-1", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-33", 0 ],
					"midpoints" : [ 110.5, 497.5, 129.5, 497.5 ],
					"order" : 1,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-50", 0 ],
					"midpoints" : [ 194.25, 497.5, 235.5, 497.5 ],
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
					"midpoints" : [ 110.5, 470.5, 110.0, 470.5 ],
					"order" : 2,
					"source" : [ "obj-1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1423.5, 397.0, 798.5, 397.0 ],
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
					"midpoints" : [ 110.5, 183.0, 110.5, 183.0 ],
					"source" : [ "obj-11", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 144.5, 362.5, 110.5, 362.5 ],
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
					"destination" : [ "obj-7", 0 ],
					"source" : [ "obj-19", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 610.5, 398.0, 798.5, 398.0 ],
					"source" : [ "obj-2", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"midpoints" : [ 76.5, 184.0, 110.5, 184.0 ],
					"source" : [ "obj-21", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1453.5, 399.1875, 798.5, 399.1875 ],
					"source" : [ "obj-22", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 396.5, 362.5, 110.5, 362.5 ],
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
					"destination" : [ "obj-22", 0 ],
					"source" : [ "obj-26", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1410.0, 510.0, 1104.25, 510.0, 1104.25, 442.04296875, 798.5, 442.04296875 ],
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
					"midpoints" : [ 133.5, 219.0, 110.5, 219.0 ],
					"source" : [ "obj-3", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1089.5, 441.0, 798.5, 441.0 ],
					"source" : [ "obj-30", 0 ]
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
					"midpoints" : [ 1243.5, 132.0, 1321.91015625, 132.0, 1321.91015625, 441.0, 798.5, 441.0 ],
					"source" : [ "obj-39", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1255.5, 397.0, 798.5, 397.0 ],
					"source" : [ "obj-40", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 1180.5, 397.0, 798.5, 397.0 ],
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
					"midpoints" : [ 1403.5, 397.5, 798.5, 397.5 ],
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
					"midpoints" : [ 376.5, 364.0, 110.5, 364.0 ],
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
					"midpoints" : [ 778.5, 397.0, 798.5, 397.0 ],
					"source" : [ "obj-62", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 965.5, 399.49609375, 798.5, 399.49609375 ],
					"source" : [ "obj-7", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-77", 0 ],
					"source" : [ "obj-70", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"midpoints" : [ 689.5, 397.0, 798.5, 397.0 ],
					"source" : [ "obj-77", 0 ]
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
					"midpoints" : [ 214.0, 308.5, 445.5, 308.5 ],
					"order" : 1,
					"source" : [ "obj-9", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-1", 0 ],
					"midpoints" : [ 110.5, 308.5, 110.5, 308.5 ],
					"order" : 0,
					"source" : [ "obj-9", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-17", 0 ],
					"midpoints" : [ 110.5, 281.5, 93.0, 281.5 ],
					"order" : 1,
					"source" : [ "obj-9", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-4", 0 ],
					"midpoints" : [ 214.0, 308.5, 463.0, 308.5 ],
					"order" : 0,
					"source" : [ "obj-9", 1 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-16" : [ "number[3]", "number[3]", 0 ],
			"obj-24" : [ "number[2]", "number[2]", 0 ],
			"obj-70" : [ "number[6]", "number[6]", 0 ],
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
