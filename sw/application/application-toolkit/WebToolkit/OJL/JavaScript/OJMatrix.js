/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

export class OJVector
{
    constructor(i, j, k)
    {
        this._values = [];
        this._values[0] = (i == null) ? 0 : i;
        this._values[1] = (j == null) ? 0 : j;
        this._values[2] = (k == null) ? 0 : k;
    }

    i() { return this._values[0]; };
    j() { return this._values[1]; };
    k() { return this._values[2]; };
}

////////////////////////////////////////////////

export class OJMatrix
{
    constructor()
    {
        this._values = [[0, 0, 0], [0, 0, 0], [0, 0, 0]];
    }

    Get(a, b)
    {
        return this._values[a][b];
    };

    GetInverseMatrix()
    {
        let out = new OJMatrix();
        let det = this.GetDeterminant();
        if (det == 0)
            return out;

        let transpose = this.GetTransposeMatrix();
        let T = transpose._values;

        out._values[0][0] = (T[1][1] * T[2][2] - (T[2][1] * T[1][2])) / det;
        out._values[0][1] = ((-1) * (T[1][0] * T[2][2] - (T[2][0] * T[1][2]))) / det;
        out._values[0][2] = (T[1][0] * T[2][1] - (T[2][0] * T[1][1])) / det;

        out._values[1][0] = ((-1) * (T[0][1] * T[2][2] - T[2][1] * T[0][2])) / det;
        out._values[1][1] = (T[0][0] * T[2][2] - T[2][0] * T[0][2]) / det;
        out._values[1][2] = ((-1) * (T[0][0] * T[2][1] - T[2][0] * T[0][1]) / det);

        out._values[2][0] = (T[0][1] * T[1][2] - T[1][1] * T[0][2]) / det;
        out._values[2][1] = ((-1) * (T[0][0] * T[1][2] - T[1][0] * T[0][2]) / det);
        out._values[2][2] = (T[0][0] * T[1][1] - T[1][0] * T[0][1]) / det;

        return out;
    };

    GetTransposeMatrix()
    {
        let T = new OJMatrix();
        for (let i = 0; i < 3; i++)
        {
            for (let j = 0; j < 3; j++)
            {
                T._values[i][j] = this._values[j][i];
            }
        }

        return T;
    };

    GetDeterminant()
    {
        let det = 0;

        det += this._values[0][0] * this._values[1][1] * this._values[2][2];
        det += this._values[0][1] * this._values[1][2] * this._values[2][0];
        det += this._values[0][2] * this._values[1][0] * this._values[2][1];
        det -= this._values[2][2] * this._values[1][0] * this._values[0][1];
        det -= this._values[2][1] * this._values[1][2] * this._values[0][0];
        det -= this._values[2][0] * this._values[1][1] * this._values[0][2];

        return det;
    }

    MultiplyVector(vector)
    {
        let result_vector = new OJVector();
        let result = result_vector._values;

        for (let i = 0; i < 3; i++)
        {
            result[i] = (this._values[i][0] * vector._values[0]) +
                        (this._values[i][1] * vector._values[1]) +
                        (this._values[i][2] * vector._values[2]);
        }

        return result_vector;
    };


    MultiplyMatrix(other_matrix)
    {
        let result_matrix = new OJMatrix();
        let result = result_matrix._values;
        let other = other_matrix._values;

        for (let r = 0; r < 3; r++)
        {
            for (let c = 0; c < 3; c++)
            {
                result[r][c] = (this._values[r][0] * other[0][c]) +
                               (this._values[r][1] * other[1][c]) +
                               (this._values[r][2] * other[2][c]);
            }
        }

        return result_matrix;
    }

    MakeIdentity()
    {
        let coeff = this._values;
        for (let i = 0; i < 3; i++)
        {
            for (let j = 0; j < 3; j++)
            {
                if (i == j)
                    coeff[i][j] = 1.0;
                else
                    coeff[i][j] = 0.0;
            }
        }
    };
}