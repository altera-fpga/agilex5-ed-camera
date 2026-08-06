/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";
import { OJMatrix } from "./OJL.js";
import { OJVector } from "./OJL.js";

let YUV_TO_G = 0;
let YUV_TO_B = 1;
let YUV_TO_R = 2;
let YUV_TO_Y = 0;
let YUV_TO_Z = 1;
let YUV_TO_X = 2;
let XYZ_TO_G = 0;
let XYZ_TO_B = 1;
let XYZ_TO_R = 2;
let RGB_TO_Y = 0;
let RGB_TO_U = 1;
let RGB_TO_V = 2;
let RGB_TO_Z = 1;
let RGB_TO_X = 2;
let XYZ_TO_Y = 0;
let XYZ_TO_U = 1;
let XYZ_TO_V = 2;
let SAMPLE_1_123 = 0;
let SAMPLE_2_123 = 1;
let SAMPLE_3_123 = 2;
let SAMPLE_123_1 = 0;
let SAMPLE_123_2 = 1;
let SAMPLE_123_3 = 2;
let G_TO_YUV = 0;
let B_TO_YUV = 1;
let R_TO_YUV = 2;
let Y_TO_YUV = 0;
let Z_TO_YUV = 1;
let X_TO_YUV = 2;
let Y_TO_XYZ = 0;
let U_TO_XYZ = 1;
let V_TO_XYZ = 2;
let G_TO_XYZ = 0;
let B_TO_XYZ = 1;
let R_TO_XYZ = 2;
let Y_TO_RGB = 0;
let Z_TO_RGB = 1;
let X_TO_RGB = 2;
let U_TO_RGB = 1;
let V_TO_RGB = 2;
let COEFF_FP_MULTI = 0x8000;

export class OJColourSpace
{
    constructor(from, to,
                colour_system, xyz_colour_system,
                composite_matrix)
    {
        this._data = new OJMatrix();

        switch (from)
        {
        case SmartImageCS.SI_CS_YUV_HD:
        case SmartImageCS.SI_CS_YUV_SD:
            switch (to)
            {
            case SmartImageCS.SI_CS_YUV_HD:
            case SmartImageCS.SI_CS_YUV_SD:
                if (from == to)
                    this._data = OJColourSpace.GetIdentity();
                else
                    this._data = OJColourSpace.GetYUVToYUVMatrix(colour_system, (to == SmartImageCS.SI_CS_YUV_HD));
                break;

            case SmartImageCS.SI_CS_RGB:
                this._data = OJColourSpace.GetYUVToRGBMatrix(colour_system, (from == SmartImageCS.SI_CS_YUV_HD));
                break;

            case SmartImageCS.SI_CS_XYZ:
                this._data = OJColourSpace.GetYUVToXYZMatrix(colour_system, (from == SmartImageCS.SI_CS_YUV_HD));
                break;

            case SmartImageCS.SI_CS_COMPOSITE:
                this._data = OJColourSpace.GetYUVToCompositeMatrix(colour_system, (from == SmartImageCS.SI_CS_YUV_HD), composite_matrix);
                break;

            default:
                break;
            }
            break;

        case SmartImageCS.SI_CS_RGB:
            switch (to)
            {
            case SmartImageCS.SI_CS_YUV_HD:
                this._data = OJColourSpace.GetRGBToYUVMatrix(colour_system, true);
                break;

            case SmartImageCS.SI_CS_YUV_SD:
                this._data = OJColourSpace.GetRGBToYUVMatrix(colour_system, false);
                break;

            case SmartImageCS.SI_CS_RGB:
                this._data = OJColourSpace.GetIdentity();
                break;

            case SmartImageCS.SI_CS_XYZ:
                this._data = OJColourSpace.GetRGBToXYZMatrix(colour_system);
                break;

            case SmartImageCS.SI_CS_COMPOSITE:
                this._data = OJColourSpace.GetRGBToCompositeMatrix(composite_matrix);
                break;

            default:
                break;
            }
            break;

        case SmartImageCS.SI_CS_XYZ:
            switch (to)
            {
            case SmartImageCS.SI_CS_YUV_HD:
                this._data = OJColourSpace.GetXYZToYUVMatrix(colour_system, xyz_colour_system, true);
                break;

            case SmartImageCS.SI_CS_YUV_SD:
                this._data = OJColourSpace.GetXYZToYUVMatrix(colour_system, xyz_colour_system, false);
                break;

            case SmartImageCS.SI_CS_RGB:
                this._data = OJColourSpace.GetXYZToRGBMatrix(colour_system);
                break;

            case SmartImageCS.SI_CS_XYZ:
                this._data = OJColourSpace.GetIdentity();
                break;

            case SmartImageCS.SI_CS_COMPOSITE:
                this._data = OJColourSpace.GetXYZToCompositeMatrix(colour_system, composite_matrix);
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
    }

    // Static function
    static GetNormalizedPrimaryMatrix(system)
    {
        let primary = new OJMatrix();
        let p = primary._values;

        let w_vector = new OJVector();
        let w = w_vector._values;

        p[RGB_TO_X][R_TO_XYZ] = system._r._x;
        p[RGB_TO_X][G_TO_XYZ] = system._g._x;
        p[RGB_TO_X][B_TO_XYZ] = system._b._x;

        p[RGB_TO_Y][R_TO_XYZ] = system._r._y;
        p[RGB_TO_Y][G_TO_XYZ] = system._g._y;
        p[RGB_TO_Y][B_TO_XYZ] = system._b._y;

        p[RGB_TO_Z][R_TO_XYZ] = 1 - (system._r._x + system._r._y);
        p[RGB_TO_Z][G_TO_XYZ] = 1 - (system._g._x + system._g._y);
        p[RGB_TO_Z][B_TO_XYZ] = 1 - (system._b._x + system._b._y);

        let invp = primary.GetInverseMatrix();
        w[RGB_TO_X] = system._w._x / system._w._y;
        w[RGB_TO_Y] = 1.0;
        w[RGB_TO_Z] = (1.0 - (system._w._x + system._w._y)) / system._w._y;

        let c = invp.MultiplyVector(w_vector); // this.MultiplyVectorByMatrix(invp, w);
        let c3 = new OJMatrix();

        c3._values[0][0] = c._values[0];
        c3._values[1][1] = c._values[1];
        c3._values[2][2] = c._values[2];

        let NPM = primary.MultiplyMatrix(c3);
        return NPM;
    }

    static GetRGBToYUVMatrix(colour_system, is_hd)
    {
        let coeff_matrix = new OJMatrix();
        let coeff = coeff_matrix._values;

        let NPM = OJColourSpace.GetNormalizedPrimaryMatrix(colour_system);
        let Wr = NPM._values[RGB_TO_Y][R_TO_YUV];
        let Wb = NPM._values[RGB_TO_Y][B_TO_YUV];
        let Wg = NPM._values[RGB_TO_Y][G_TO_YUV];

        // RGB -> YUV Coefficients
        coeff[RGB_TO_Y][R_TO_YUV] = Wr;
        coeff[RGB_TO_Y][G_TO_YUV] = Wg;
        coeff[RGB_TO_Y][B_TO_YUV] = Wb;
        coeff[RGB_TO_U][R_TO_YUV] = (-0.5 * Wr) / (1.0 - Wb);
        coeff[RGB_TO_U][G_TO_YUV] = (-0.5 * Wg) / (1.0 - Wb);
        coeff[RGB_TO_U][B_TO_YUV] = 0.5;
        coeff[RGB_TO_V][R_TO_YUV] = 0.5;
        coeff[RGB_TO_V][G_TO_YUV] = (-0.5 * Wg) / (1.0 - Wr);
        coeff[RGB_TO_V][B_TO_YUV] = (-0.5 * Wb) / (1.0 - Wr);

        return coeff_matrix;
    }

    static GetYUVToRGBMatrix(colour_system, is_hd)
    {
        let A = OJColourSpace.GetRGBToYUVMatrix(colour_system, is_hd);
        return A.GetInverseMatrix();
    }

    static GetYUVToCompositeMatrix(colour_system,
                                   is_hd,
                                   composite_matrix_type)
    {
        let B = OJColourSpace.GetYUVToRGBMatrix(colour_system, is_hd);
        let A = OJColourSpace.GetRGBToCompositeMatrix(composite_matrix_type);

        return A.Multiply(B);
    }

    static GetRGBToXYZMatrix(colour_system)
    {
        return OJColourSpace.GetNormalizedPrimaryMatrix(colour_system);
    }

    static GetRGBToCompositeMatrix(composite_matrix_type)
    {
        let result = new OJMatrix();
        // RGB -> Composite (yuv) Coefficients
        let coeff = result._values;

        if (composite_matrix == CompositeMatrixType.M_NTSC_MN_PAL)
        {
            // (M) NTSC, (M, N) PAL
            coeff[0][0] =  0.297; coeff[0][1] =  0.058; coeff[0][2] =  0.151;
            coeff[1][0] = -0.147; coeff[1][1] =  0.221; coeff[1][2] = -0.074;
            coeff[2][0] = -0.261; coeff[2][1] = -0.051; coeff[2][2] =  0.312;
        }
        else if (composite_matrix == CompositeMatrixType.J_NTSC)
        {
            // (J) NTSC
            coeff[0][0] =  0.321; coeff[0][1] =  0.062; coeff[0][2] =  0.164;
            coeff[1][0] = -0.159; coeff[1][1] =  0.239; coeff[1][2] = -0.080;
            coeff[2][0] = -0.282; coeff[2][1] = -0.055; coeff[2][2] =  0.337;
        }
        else if (composite_matrix == CompositeMatrixType.BDGHINC_PAL)
        {
            // (B, D, G, H, I, NC) PAL
            coeff[0][0] =  0.314; coeff[0][1] =  0.061; coeff[0][2] =  0.160;
            coeff[1][0] = -0.155; coeff[1][1] =  0.234; coeff[1][2] = -0.079;
            coeff[2][0] = -0.275; coeff[2][1] = -0.054; coeff[2][2] =  0.329;
        }

        return result;
    }

    static GetXYZToCompositeMatrix(colour_system,
                                   composite_matrix_type)
    {
        let B = OJColourSpace.GetXYZToRGBMatrix(colour_system);
        let A = OJColourSpace.GetRGBToCompositeMatrix(composite_matrix_type);
        return A.Multiply(B);
    }

    static GetXYZToRGBMatrix(colour_system)
    {
        let A = OJColourSpace.GetNormalizedPrimaryMatrix(colour_system);
        return A.GetInverseMatrix();
    }

    static GetXYZToYUVMatrix(colour_system, xyz_colour_system, is_hd)
    {
        let B = OJColourSpace.GetXYZToRGBMatrix(xyz_colour_system);
        let A = OJColourSpace.GetRGBToYUVMatrix(colour_system, is_hd);
        return A.Multiply(B);
    }

    // Static function
    static GetYUVToXYZMatrix(colour_system, is_hd)
    {
        let A = OJColourSpace.GetRGBToXYZMatrix(colour_system);
        let B = OJColourSpace.GetYUVToRGBMatrix(colour_system, is_hd);
        return A.Multiply(B);
    }

    static GetYUVToYUVMatrix(colour_system, is_hd)
    {
        let B = OJColourSpace.GetYUVToRGBMatrix(colour_system, !is_hd);
        let A = OJColourSpace.GetRGBToYUVMatrix(colour_syste, is_hd);
        return A.Multiply(B);
    }

    static GetIdentity()
    {
        let identity_matrix = new OJMatrix();
        identity_matrix.MakeIdentity();
        return identity_matrix;
    }
}

//////////////////////

class XY
{
    constructor(x, y)
    {
        this._x = x;
        this._y = y;
    }
}

const ColourSystemID =
{
	CSID_UNKNOWN : "unknown",
	CSID_DCI_GAMUT : "DCI-P3 Gamut",
	CSID_REC_709_GAMUT : "Rec. 709 Gamut",
	CSID_EBU_GAMUT : "EBU Gamut",
	CSID_SMPTE_C_GAMUT : "SMPTE C Gamut",
	CSID_REC_2020_GAMUT : "Rec. 2020 Gamut",
	CSID_S_GAMUT3_CINE : "S-Gamut3.Cine",
	CSID_S_GAMUT3 : "S-Gamut3",
	CSID_S_GAMUT : "S-Gamut",
	CSID_ACES_GAMUT : "ACES-Gamut"
}

export class OJColourSystem
{
    constructor(colour_system_id)
    {
        this._colour_system_id = colour_system_id;
        this._gamut_title = "";

        if (colour_system_id == ColourSystemID.CSID_UNKNOWN)
        {
            this._r = new XY(0, 0);
            this._g = new XY(0, 0);
            this._b = new XY(0, 0);
            this._w = new XY(0, 0);
        }
        else if (colour_system_id == ColourSystemID.CSID_DCI_GAMUT)
        {
            this._gamut_title = "DCI-P3 Gamut";
            this._r = new XY(0.680, 0.320); // Page 2
            this._g = new XY(0.2650, 0.690); // Page 2
            this._b = new XY(0.150, 0.060); // Page 2
            this._w = new XY(0.3140, 0.3510);  // White D-65
        }
        else if (colour_system_id == ColourSystemID.CSID_REC_709_GAMUT)
        {
            this._gamut_title = "Rec. 709 Gamut";
            this._r = new XY(0.640, 0.330); // Page 2
            this._g = new XY(0.300, 0.600); // Page 2
            this._b = new XY(0.150, 0.060); // Page 2
            this._w = new XY(0.3127, 0.3290);  // White D-65
        }
        else if (colour_system_id == ColourSystemID.CSID_EBU_GAMUT)
        {
            this._gamut_title = "EBU Gamut";
            this._r = new XY(0.640, 0.330); //Page 9
            this._g = new XY(0.290, 0.600); //Page 9
            this._b = new XY(0.15, 0.060); //Page 9
            this._w = new XY(0.3127, 0.3290);  // White D-65
        }
        else if (colour_system_id == ColourSystemID.CSID_SMPTE_C_GAMUT)
        {
            this._gamut_title = "SMPTE C Gamut";
            this._r = new XY(0.630, 0.340); //Page 3
            this._g = new XY(0.310, 0.595); //Page 3
            this._b = new XY(0.155, 0.070); //Page 3
            this._w = new XY(0.3127, 0.3290);  // White D-65
        }
        else if (colour_system_id == ColourSystemID.CSID_REC_2020_GAMUT)
        {
            this._gamut_title = "Rec. 2020 Gamut"; // ITU 2020, 2008 (DataSheets\Standards\ITU-CCIR)
            this._r = new XY(0.708, 0.292); // Page 3
            this._g = new XY(0.170, 0.797); // Page 3
            this._b = new XY(0.131, 0.046); // Page 3
            this._w = new XY(0.3127, 0.3290);  // White D-65
        }
        else if (colour_system_id == ColourSystemID.CSID_S_GAMUT3_CINE)
        {
            this._gamut_title = "S-Gamut3.Cine"; // Sony S-Gamut3.Cine (https://www.sony.co.uk/pro/support/attachment/1237494271390/1237494271406/technical-summary-for-s-gamut3-cine-s-log3-and-s-gamut3-s-log3.pdf?token=GnxHJr_5IvMg2RMB2M33cMfL9jDvz-4CXhR2my0z_AyUdeahFLKO9EVcPQrZeMBiudbMJZW53x8JmSagDRejgUsQzhGzfltcjO06c-3aRlYRyy0o7IQnCzUYOhSZkpv0EMY6H192pFoB_mgAXjsXKVNr9OA.)
            this._r = new XY(0.766, 0.275);
            this._g = new XY(0.225, 0.800);
            this._b = new XY(0.089, -0.087);
            this._w = new XY(0.3127, 0.3290);
        }
        else if (colour_system_id == ColourSystemID.CSID_S_GAMUT3)
        {
            this._gamut_title = "S-Gamut3"; // Sony S-Gamut3 (https://www.sony.co.uk/pro/support/attachment/1237494271390/1237494271406/technical-summary-for-s-gamut3-cine-s-log3-and-s-gamut3-s-log3.pdf?token=GnxHJr_5IvMg2RMB2M33cMfL9jDvz-4CXhR2my0z_AyUdeahFLKO9EVcPQrZeMBiudbMJZW53x8JmSagDRejgUsQzhGzfltcjO06c-3aRlYRyy0o7IQnCzUYOhSZkpv0EMY6H192pFoB_mgAXjsXKVNr9OA.)
            this._r = new XY(0.730, 0.280);
            this._g = new XY(0.140, 0.855);
            this._b = new XY(0.100, -0.050);
            this._w = new XY(0.3127, 0.3290);  // White D-65
        }
        else if (colour_system_id == ColourSystemID.CSID_S_GAMUT)
        {
            this._gamut_title = "S-Gamut"; // Sony S-Gamut (https://www.sony.co.uk/pro/support/attachment/1237494271390/1237494271406/technical-summary-for-s-gamut3-cine-s-log3-and-s-gamut3-s-log3.pdf?token=GnxHJr_5IvMg2RMB2M33cMfL9jDvz-4CXhR2my0z_AyUdeahFLKO9EVcPQrZeMBiudbMJZW53x8JmSagDRejgUsQzhGzfltcjO06c-3aRlYRyy0o7IQnCzUYOhSZkpv0EMY6H192pFoB_mgAXjsXKVNr9OA.)
            this._r = new XY(0.730, 0.280);
            this._g = new XY(0.140, 0.855);
            this._b = new XY(0.100, -0.050);
            this._w = new XY(0.3127, 0.3290);  // White D-65
        }
        else if (colour_system_id == ColourSystemID.CSID_ACES_GAMUT)
        {
            this._gamut_title = "ACES-Gamut"; // ACES-Gamut (https://www.sony.co.uk/pro/support/attachment/1237494271390/1237494271406/technical-summary-for-s-gamut3-cine-s-log3-and-s-gamut3-s-log3.pdf?token=GnxHJr_5IvMg2RMB2M33cMfL9jDvz-4CXhR2my0z_AyUdeahFLKO9EVcPQrZeMBiudbMJZW53x8JmSagDRejgUsQzhGzfltcjO06c-3aRlYRyy0o7IQnCzUYOhSZkpv0EMY6H192pFoB_mgAXjsXKVNr9OA.)
            this._r = new XY(0.7347, 0.2653);
            this._g = new XY(0.0000, 1.0000);
            this._b = new XY(0.00010, -0.077);
            this._w = new XY(0.3168, 0.333767);  // White (Approx.D-60)
        }
    }

    FromJson(json)
    {
        this._gamut_title = json._title;
        this._r = new XY(json.red_x, json.red_y);
        this._g = new XY(json.green_x, json.green_y);
        this._b = new XY(json.blue_x, json.blue_y);
        this._w = new XY(json.white_x, json.white_y);
    }

    GetNormalizedPrimaryMatrix()
    {
        return OJColourSpace.GetNormalizedPrimaryMatrix(this);
    }

    GetColourSystemName()
    {
        // Called must not free this string
        return this._gamut_title;
    }

    SetCustom(custom_name)
    {
        this._gamut_title = custom_name;
        this._colour_system_id = CSID_CUSTOM;
    }

    static GetColourSystemNameFromID(colour_system_id)
    {
        let name = "";
        if (colour_system_id == ColourSystemID.CSID_UNKNOWN)
            name = "Unknown";
        else if (colour_system_id == ColourSystemID.CSID_DCI_GAMUT)
            name = "DCI-P3 Gamut";
        else if (colour_system_id == ColourSystemID.CSID_REC_709_GAMUT)
            name = "Rec. 709 Gamut";
        else if (colour_system_id == ColourSystemID.CSID_EBU_GAMUT)
            name = "EBU Gamut";
        else if (colour_system_id == ColourSystemID.CSID_SMPTE_C_GAMUT)
            name = "SMPTE C Gamut";
        else if (colour_system_id == ColourSystemID.CSID_REC_2020_GAMUT)
            name = "Rec. 2020 Gamut";
        else if (colour_system_id == ColourSystemID.CSID_S_GAMUT3_CINE)
            name = "S Gamut3 Cine";
        else if (colour_system_id == ColourSystemID.CSID_S_GAMUT3)
            name = "S Gamut3";
        else if (colour_system_id == ColourSystemID.CSID_S_GAMUT)
            name = "S Gamut";
        else if (colour_system_id == ColourSystemID.CSID_ACES_GAMUT)
            name = "ACES Gamut";
        else if (colour_system_id == ColourSystemID.CSID_CUSTOM)
            name = "Custom Gamut";

        return name;
    };

    InGamut(x, y)
    {
        let Or1 = Orientation(this._r, this._g, x, y);
        let Or2 = Orientation(this._g, this._b, x, y);
        let Or3 = Orientation(this._b, this._r, x, y);

        if ((Or1 == Or2) && (Or2 == Or3))
            return true;
        else if (Or1 == 0)
            return ((Or2 == 0) || (Or3 == 0));
        else if (Or2 == 0)
            return ((Or1 == 0) || (Or3 == 0));
        else if (Or3 == 0)
            return ((Or2 == 0) || (Or1 == 0));
        else
            return false;
    }

    static UnknownColourSystem = new OJColourSystem(ColourSystemID.CSID_SMPTE_C_GAMUT);

    // SMPTE 170M/SMPTE C
    static SMPTE_C_NTSC_ColourSystem = new OJColourSystem(ColourSystemID.CSID_SMPTE_C_GAMUT);

    // NTSC-J Colour Primaries
    static JAPAN_NTSC_ColourSystem = new OJColourSystem(ColourSystemID.CSID_SMPTE_C_GAMUT);

    // ITU REC BT470-6
    static EBU_PALColourSystem = new OJColourSystem(ColourSystemID.CSID_EBU_GAMUT);

    // ITU REC BT709
    static Rec709ColourSystem = new OJColourSystem(ColourSystemID.CSID_REC_709_GAMUT);

    // DCI Gamut
    static DCIColourSystem = new OJColourSystem(ColourSystemID.CSID_DCI_GAMUT);

    // Rec 2020 Gamut
    static Rec2020ColourSystem = new OJColourSystem(ColourSystemID.CSID_REC_2020_GAMUT);

    // S-Gamut3.Cine
    static S_Gamut3_CineColourSystem = new OJColourSystem(ColourSystemID.CSID_S_GAMUT3_CINE);

    // S-Gamut3
    static S_Gamut3ColourSystem = new OJColourSystem(ColourSystemID.CSID_S_GAMUT3);

    // S-Gamut
    static S_GamutColourSystem = new OJColourSystem(ColourSystemID.CSID_S_GAMUT);

    // ACES-Gamut
    static ACES_GamutColourSystem = new OJColourSystem(ColourSystemID.CSID_ACES_GAMUT);
}

function Orientation(a, b, px, py)
{
	// Linear determinant of the 3 points
	let Orin = (b._x - a._x) * (py - a._y) - (px - a._x) * (b._y - a._y);

	if (Orin > 0)
		return 1;		// Orientaion is to the right-hand side
	else if (Orin < 0)
		return -1;		// Orientaion is to the left-hand side
	else
		return 0;		// Orientaion is neutral aka collinear
}

