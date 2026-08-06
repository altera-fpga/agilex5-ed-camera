/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

///////////////////////////////////////////////////////////////////////////////
// SVG helper
///////////////////////////////////////////////////////////////////////////////
export var wsvg = (function(){
    return {
        create: function(elem)
        {
            return document.createElementNS("http://www.w3.org/2000/svg", elem);			
        },

        path_data: function(d)
        {
            var v = "";
    
            if(d.length > 0)
            {
                let x = d[0][0];
                let y = d[0][1];
                v = "M " + x + " " + y;
    
                for(var i = 1; i < d.length; ++i)
                {
                    let x = d[i][0];
                    let y = d[i][1];
        
                    v += (" L" + x + " " + y);
                }
            }
    
            return v;			
        },

        range: function(start, stop, step)
        {
            var r = [];
            for(var i = 0; i < stop; i += step)
                r.push(i);
            return r;
        },

        range_inclusive: function(start, stop, step)
        {
            var r = [];
            for(var i = 0; i <= stop; i += step)
                r.push(i);
            return r;
        },

        drag: function()
        {
            return {	
                add_svg: function(child, callback, get_scale_cb)
                {
                    var svg = child;

                    while (svg && svg.tagName != 'svg') 
                        svg = svg.parentNode;

                    if(!svg) 
                        return console.error(child, "must be an SVG element!");

                    var parent = document.documentElement || svg;

                    function get_svg_pos(e)
                    {
                        var ctm = svg.getScreenCTM();
                        return [((e.clientX - ctm.e) / ctm.a), ((e.clientY - ctm.f) / ctm.d)];
                    };

                    child.onmousedown = function(e)
                    {
                        child.setAttribute("stroke", "#0062aa");
                        child.setAttribute("fill", "#0062aa");            
            
                        child._mouse_down_pos = get_svg_pos(e);
                        child._mouse_down_d = child._d.slice();
                        let scale = 1.0;
                        if (get_scale_cb)
                            scale = get_scale_cb();
                                                    
                        parent.onmousemove = function(e)
                        {
                            let pos = get_svg_pos(e);
                            let dx = (pos[0] - child._mouse_down_pos[0]) / scale;
                            let dy = (pos[1] - child._mouse_down_pos[1]) / scale;
    
                            child._d[0] = child._mouse_down_d[0] + dx;
                            child._d[1] = child._mouse_down_d[1] + dy;

                            e.preventDefault();

                            if(callback)
                                callback(child);
                        };

                        parent.onmouseup = function(e)
                        {
                            parent.onmousemove = null;
                        }

                        if(callback)
                            callback(child);
                    }				
                }
            };
        }
    };
})();


///////////////////////////////////////////////////////////////////////////////
// Math utils
///////////////////////////////////////////////////////////////////////////////

export var wmath = (function()
{

    // Lower-upper decomposition for transformation matrix
    function LUD(a_in)
    {
        var LU_DIM = 8;
        var k = LU_DIM - 1;
    
        var p = [];
    
        for(var s = 0; s < LU_DIM; ++s)
        {
            var f = s;
            var a = a_in[s];
            var c = Math.abs(a[s]);
    
            for(var i = s + 1; i < LU_DIM; ++i)
            {
                var o = Math.abs(a_in[i][s]);
    
                if(c < o)
                {
                    c = o;
                    f = i;
                }
            }
    
            p.push(f);
    
            if(f != s)
            {
                a_in[s] = a_in[f];
                a_in[f] = a;
                a = a_in[s];
            }
    
            var u = a[s];
    
            for(var r = s + 1; r < LU_DIM; ++r)
            {
                a_in[r][s] /= u;
                var l = a_in[r];
    
                var i = s + 1;
    
                while(i < k)
                {
                    l[i]-=l[s]*a[i];
                    ++i;
                    l[i]-=l[s]*a[i];
                    ++i;
                }
    
                if(i == k)
                    l[i] -= l[s] * a[i];
            }
        }
    
        return {_a:a_in, _p:p};
    }

    function LUDSolve(lu, m)
    {
        var LU_DIM = 8;

        for(var i = 0; i < LU_DIM; ++i)
        {
            var f = lu._p[i];

            if(lu._p[i] != i)
            {
                var h = m[i];
                m[i] = m[f];
                m[f] = h;
            }

            for(var j = 0; j < i ; ++j)
                m[i] -= m[j] * lu._a[i][j];
        }

        for(var i = LU_DIM - 1; i >= 0; --i)
        {
            for(var j = i + 1; j < LU_DIM; ++j)
                m[i] -= m[j] * lu._a[i][j];

            m[i] /= lu._a[i][i];
        }

        return m;
    }

    return {
        vector_cross_product: function(v0, v1)
        {
            return [
                (v0[1] * v1[2]) - (v0[2] * v1[1]),
                (v0[2] * v1[0]) - (v0[0] * v1[2]),
                (v0[0] * v1[1]) - (v0[1] * v1[0]),
                0.0
            ];
        },
        
        vector_dot_product: function(v0, v1)
        {
            return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
        },

        matrix_vector_product: function(m, v)
        {           
            return [
                m[0] * v[0] + m[4] * v[1] + m[8 ] * v[2] + m[12] * v[3],
                m[1] * v[0] + m[5] * v[1] + m[9 ] * v[2] + m[13] * v[3],
                m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3],
                m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3]
            ];
        },

        matrix_vector_column_product: function(m, v)
        {
            return [
                m[0] *  v[0] + m[1] *  v[1] + m[2] *  v[2] + m[3] *  v[3],
                m[4] *  v[0] + m[5] *  v[1] + m[6] *  v[2] + m[7] *  v[3],
                m[8] *  v[0] + m[9] *  v[1] + m[10] * v[2] + m[11] * v[3],
                m[12] * v[0] + m[13] * v[1] + m[14] * v[2] + m[15] * v[3]
            ];
        },

        matrix_product: function (a, b)
        {
            return [
                (a[0] * b[ 0] + a[4] * b[ 1] + a[8 ] * b[ 2] + a[12] * b[ 3]),
                (a[1] * b[ 0] + a[5] * b[ 1] + a[9 ] * b[ 2] + a[13] * b[ 3]),
                (a[2] * b[ 0] + a[6] * b[ 1] + a[10] * b[ 2] + a[14] * b[ 3]),
                (a[3] * b[ 0] + a[7] * b[ 1] + a[11] * b[ 2] + a[15] * b[ 3]),
            
                (a[0] * b[ 4] + a[4] * b[ 5] + a[8 ] * b[ 6] + a[12] * b[ 7]),
                (a[1] * b[ 4] + a[5] * b[ 5] + a[9 ] * b[ 6] + a[13] * b[ 7]),
                (a[2] * b[ 4] + a[6] * b[ 5] + a[10] * b[ 6] + a[14] * b[ 7]),
                (a[3] * b[ 4] + a[7] * b[ 5] + a[11] * b[ 6] + a[15] * b[ 7]),
            
                (a[0] * b[ 8] + a[4] * b[ 9] + a[8 ] * b[10] + a[12] * b[11]),
                (a[1] * b[ 8] + a[5] * b[ 9] + a[9 ] * b[10] + a[13] * b[11]),
                (a[2] * b[ 8] + a[6] * b[ 9] + a[10] * b[10] + a[14] * b[11]),
                (a[3] * b[ 8] + a[7] * b[ 9] + a[11] * b[10] + a[15] * b[11]),
            
                (a[0] * b[12] + a[4] * b[13] + a[8 ] * b[14] + a[12] * b[15]),
                (a[1] * b[12] + a[5] * b[13] + a[9 ] * b[14] + a[13] * b[15]),
                (a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15]),
                (a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15])
            ];
        },

        transformation_matrix: function(sourcePoints, targetPoints)
        {
            function Solve(a, m){
                return LUDSolve(LUD(a), m);
            }

            for(var a = [], m = [], i = 0; i < 4; ++i)
            {
                var s = sourcePoints[i];
                var t = targetPoints[i];

                a.push([s[0], s[1], 1, 0, 0, 0, -s[0] * t[0], -s[1] * t[0]]);
                m.push(t[0]);

                a.push([0, 0, 0, s[0], s[1], 1, -s[0] * t[1], -s[1] * t[1]]);
                m.push(t[1]);
            }

            var x = Solve(a, m);

            var matrix = [
                x[0], x[3], 0, x[6],
                x[1], x[4], 0, x[7],
                0,    0, 1,    0,
                x[2], x[5], 0,    1
                ];

            return matrix;
        },

        rotation_matrix: function(yaw, pitch, roll)
        {
            var matrix_rx = [ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 ];
            var matrix_ry = [ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 ];
            var matrix_rz = [ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 ];
        
            var cos_theta = 0.0;
            var sin_theta = 0.0;
        
            // Yaw
            cos_theta = Math.cos(yaw);
            sin_theta = Math.sin(yaw);
        
            matrix_rz[0] = cos_theta;
            matrix_rz[1] = -sin_theta;
            matrix_rz[4] = sin_theta;
            matrix_rz[5] = cos_theta;
        
            // Pitch
            cos_theta = Math.cos(pitch);
            sin_theta = Math.sin(pitch);
        
            matrix_ry[0] = cos_theta;
            matrix_ry[2] = sin_theta;
            matrix_ry[8] = -sin_theta;
            matrix_ry[10] = cos_theta;
        
            // Roll
            cos_theta = Math.cos(roll);
            sin_theta = Math.sin(roll);
        
            matrix_rx[5] = cos_theta;
            matrix_rx[6] = -sin_theta;
            matrix_rx[9] = sin_theta;
            matrix_rx[10] = cos_theta;
        
            var m = wmath.matrix_product(matrix_rx, matrix_ry);
            return m;
        },

        plane_from_three_points: function(p0, p1, p2)
        {
            var v0 = [
                p1[0] - p0[0],
                p1[1] - p0[1],
                p1[2] - p0[2],
                p1[3] - p0[3]
            ];

            var v1 = [
                p2[0] - p0[0],
                p2[1] - p0[1],
                p2[2] - p0[2],
                p2[3] - p0[3]
            ];

            var plane = {
                _n: wmath.vector_cross_product(v0, v1),
                _p: [p0[0], p0[1], p0[2], p0[3]]
            };

            return plane;
        },

        plane_line_intersection: function(plane, p0, p1)
        {
            var l = [
                p1[0] - p0[0],
                p1[1] - p0[1],
                p1[2] - p0[2],
                p1[3] - p0[3]
            ];
        
            function value_not_zero(v){
                return (Math.abs(v - 0.0) > 0.000001);
            };
        
            var prod1 = wmath.vector_dot_product(l, plane._n);
        
            if(value_not_zero(prod1))
            {
                var diff = [
                    p0[0] - plane._p[0],
                    p0[1] - plane._p[1],
                    p0[2] - plane._p[2],
                    p0[3] - plane._p[3]
                ];
        
                var prod2 = wmath.vector_dot_product(diff, plane._n);
                var prod3 = prod2 / prod1;
        
                return [
                    p0[0] - l[0] * prod3,
                    p0[1] - l[1] * prod3,
                    p0[2] - l[2] * prod3,
                    p0[3] - l[3] * prod3
                ];
            }		
        },

        apply_spline: function(pts, num_segments){

            var tension = 0.5;

            var knots = [];
            for(var i = 0; i  <pts.length; i += num_segments)
                knots.push(pts[i]);

            // Applying a spline requires a preceeding and a following
            // knot for each actual knot on the curve. Therefore we need to
            // add an extra knot before the first and after the last one.
            // We simply mirror the position
            var p = [];
            p[0] = knots[0][0]-(knots[1][0]-knots[0][0]);
            p[1] = knots[0][1]-(knots[1][1]-knots[0][1]);
            knots.unshift(p);
            
            p = [];
            p[0] = knots[knots.length - 1][0] + (knots[knots.length - 1][0]-knots[knots.length - 2][0]);
            p[1] = knots[knots.length - 1][1] + (knots[knots.length - 1][1]-knots[knots.length - 2][1]);
            knots.push(p);
        
            // Iterate over each actual knot
            for(var i = 1; i < (knots.length - 2); ++i)
            {	
                // calc tension vectors
                var t1x = (knots[i+1][0] - knots[i-1][0]) * tension;
                var t2x = (knots[i+2][0] - knots[i][0]) * tension;
        
                var t1y = (knots[i+1][1] - knots[i-1][1]) * tension;
                var t2y = (knots[i+2][1] - knots[i][1]) * tension;
                
                // Iterate over each sub segment
                for(var t = 0; t <= num_segments; ++t)
                {
                    // calc step
                    var st = t / num_segments;
                    var st_pow2 = st * st;
                    var st_pow3 = st_pow2 * st;
        
                    // calc cardinals
                    var c1 =   2 * st_pow3  - 3 * st_pow2 + 1; 
                    var c2 = -(2 * st_pow3) + 3 * st_pow2; 
                    var c3 =       st_pow3  - 2 * st_pow2 + st; 
                    var c4 =       st_pow3  -     st_pow2;
        
                    // calc x and y cords with common control vectors
                    var x = c1 * knots[i][0]  + c2 * knots[i+1][0] + c3 * t1x + c4 * t2x;
                    var y = c1 * knots[i][1]  + c2 * knots[i+1][1] + c3 * t1y + c4 * t2y;
                    
                    var idx = ((i - 1) * num_segments) + t;
                    pts[idx][0] = x;
                    pts[idx][1] = y;
                }
            }    
        
            return pts;
        }
    };
})();

///////////////////////////////////////////////////////////////////////////////
// SVG helper
///////////////////////////////////////////////////////////////////////////////

export var wtransform = (function()
{
    return {
        radial: function(){
            var _focus = [0, 0];
            var _k1 = 0;
            var _k2 = 0;

            //var _radius = 1.0;
            var _width = 1;
            var _height = 1;
            var _ratio = 16/9;

            function normalise_pos(pos){
                return [pos[0] / _width, (pos[1] / _height) * _ratio];
            };

            function restore_pos(pos){
                return [pos[0] * _width, (pos[1] / _ratio) * _height];
            };

            function transform(p){
                p = normalise_pos(p);
                var dx = p[0] - _focus[0];
                var dy = p[1] - _focus[1];

                var r_pow2 = (dx * dx + dy * dy);
                var m = r_pow2 * _k1;
                var r_pow4 = r_pow2 * r_pow2;
                m += r_pow4 * _k2;
            
                var x = p[0] + dx * m;
                var y = p[1] + dy * m;
            
                return restore_pos([x, y]);
            }

            transform.focus = function(v){
                _focus = normalise_pos(v);
            };

            transform.radius = function(v){
                _radius = v;
            };

            transform.resolution = function(width, height){
                _ratio = height /width;
                _width = width;
                _height = height;
            };

            transform.strength = function(v){
                _k1 = v;
            }

            return transform;
        },

        perspective: function(){
            var img_width = 1;
            var img_height = 1;
            var sourcePoints = [[0, 0], [img_width, 0], [0, img_height], [img_width, img_height]];
            var targetPoints = [[0, 0], [img_width, 0], [0, img_height], [img_width, img_height]];			
        
            var matrix = wmath.transformation_matrix(sourcePoints, targetPoints);

            function transform(p)
            {
                //4x4 matrix in column-major, 4x1 column vector
                var point = wmath.matrix_vector_product(matrix, [p[0], p[1], 0, 1]);
                return [point[0] / point[3], point[1] / point[3]];
            }

            transform.source_points = function(sp){
                sourcePoints = Array.from(sp);
                targetPoints = Array.from(sp);
            }

            transform.set_point = function(idx, p){
                if(idx < targetPoints.length){
                    targetPoints[idx] = p;
                    matrix = wmath.transformation_matrix(sourcePoints, targetPoints);
                }
            }

            transform.target_points = function(tp){
                targetPoints = Array.from(tp);
                matrix = wmath.transformation_matrix(sourcePoints, targetPoints);
            }			

            return transform;
        },

        affine: function(){
            // Zoom and Rotate origin
            var origin = [0,0];
            
            // Translate and restore matrices
            // For scaling and rotating around particular point
            var hoff,voff;
            var mt1;
            var mt2;
            
            // Mirror
            var _hmirr, _vmirr;
            // Zoom
            var _hzoom,_vzoom,_zoom;
            var mz;
            
            // Rotate
            var theta;
            var mr;
            
            // Combined matrix
            var matrix;

            function recalculate_matrix()
            {
                // Translate rotate and restore
                // using one matrix which is a product
                // of three consequtive transforms
                var                
                m = wmath.matrix_product(mt1, mr);
                m = wmath.matrix_product(m, mz);
                m = wmath.matrix_product(m, mt2);
                
                // Arrange in column major order
                matrix = m;			
            }			

            function reset_private()
            {
                var mid = [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1];
                hoff = 0;
                voff = 0;
                mt1 = Array.from(mid);
                mt1[12] = origin[0];
                mt1[13] = origin[1];
                mt2 = Array.from(mid);
                mt2[12] = -origin[0];
                mt2[13] = -origin[1];
                _hmirr = 1;
                _vmirr = 1;
                _hzoom = 1;
                _vzoom = 1;
                _zoom = 1;
                mz = Array.from(mid);
                theta = 0;
                mr = Array.from(mid);
                recalculate_matrix();
            }

            function update_zoom_matrix()
            {
                mz[0] = (_hmirr * _hzoom * _zoom);
                mz[5] = (_vmirr * _vzoom *_zoom);
            }

            function transform(point, idx)
            {
                var vector = [point[0],point[1],0,1];
                var result = wmath.matrix_vector_product(matrix, vector);
                return[result[0],result[1]];
            }

            transform.set_h_offset = function(v)
            {
                hoff = v;				
                mt1[12] = origin[0]+hoff;			
                recalculate_matrix();
            }
            
            transform.set_v_offset = function(v)
            {
                voff = v;		
                mt1[13] = origin[1]+voff;		
                recalculate_matrix();
            }		
            
            transform.set_zoom = function(v)
            {
                _zoom = v;
                update_zoom_matrix();
                recalculate_matrix();
            }
            
            transform.set_h_zoom = function(v)
            {
                _hzoom = v;
                update_zoom_matrix();
                recalculate_matrix();
            }
            
            transform.set_v_zoom = function(v)
            {
                _vzoom = v;
                update_zoom_matrix();
                recalculate_matrix();
            }		
            
            transform.set_h_mirror = function(v)
            {
                _hmirr = v ? -1.0 : 1.0;
            
                update_zoom_matrix();
                recalculate_matrix();
            }
            
            transform.set_v_mirror = function(v)
            {
                _vmirr = v ? -1.0 : 1.0;
            
                update_zoom_matrix();
                recalculate_matrix();
            }

            transform.set_rotate = function(v)
            {
                theta = v * (Math.PI/180);
                var cos_theta = Math.cos(theta);
                var sin_theta = Math.sin(theta);
                mr[0] = cos_theta;
                mr[1] = -sin_theta;
                mr[4] = sin_theta;
                mr[5] = cos_theta;
                recalculate_matrix();
            }
            
            transform.GetBoundingRotated = function(rect, theta)
            {
                var cos_theta = Math.abs(Math.cos(theta));
                var sin_theta = Math.abs(Math.sin(theta));

                var m = [cos_theta,sin_theta,0, sin_theta,cos_theta,0,0,0,1];
                
                var vector = [rect[0],rect[1],1];
                
                var result = [
                        m[0] * vector[0] + m[3] * vector[1] + m[6] * vector[2],
                        m[1] * vector[0] + m[4] * vector[1] + m[7] * vector[2],
                        m[2] * vector[0] + m[5] * vector[1] + m[8] * vector[2],
                      ];
                
                return[result[0],result[1]];
            }
            
            transform.reset = function()
            {
                reset_private();
            }

            transform.origin = function(v)
            {
                origin = v;

                mt1[12] = origin[0];
                mt1[13] = origin[1];

                mt2[12] = -origin[0];
                mt2[13] = -origin[1];

                recalculate_matrix();
            }
            
            reset_private();
            return transform;			
        },

        keystone: function()
        {
            var _img_width = 1;
            var _img_height = 1;

            var _hkey = 0 * (Math.PI/180);
            var _vkey = 0 * (Math.PI/180);
            var _FOV = 30 * (Math.PI/180);
            var _voff = 0;

            //var _points = [[0, 0], [_img_width, 0], [_img_width, _img_height], [0, _img_height]];
            var _points = [[0, 0], [_img_width, 0], [0, _img_height], [_img_width, _img_height]];

            function update_points()
            {
                var pitch = _hkey;
                var roll = _vkey;
            
                var half_w = _img_width / 2.0;
                var half_h = _img_height / 2.0;
            
                var voff = _voff * _img_height;
                var top = -half_h - voff;
                var bot = half_h - voff;
                //var src_points = [[-half_w, top],[half_w, top],[half_w, bot],[-half_w, bot],[0.0, -voff]];
                var src_points = [[-half_w, top],[half_w, top],[-half_w, bot],[half_w, bot],[0.0, -voff]];
            
                // Focal length (z coordinate)
                var r = Math.sqrt(half_w * half_w + half_h * half_h);
                var source_z = r / Math.tan(_FOV/2.0);
            
                var rmatrix = wmath.rotation_matrix(0.0, pitch, roll);

                var source_vectors = src_points.map(function(p){
                    return [p[0], p[1], source_z, 1.0];
                });
            
                // Get projection plane from first three points
                var source_plane = wmath.plane_from_three_points(source_vectors[0], source_vectors[1], source_vectors[2]);
                var origin_vector = [0.0, 0.0, 0.0, 1.0];

                var dst_points = source_vectors.map(function(v){
                    var rotated_vector = wmath.matrix_vector_column_product(rmatrix, v);
                    var intersection_vector = wmath.plane_line_intersection(source_plane, origin_vector, rotated_vector);
                    return [intersection_vector[0], intersection_vector[1]];
                });
            
                // Recenter projection for better transform accuracy
                var NUM_POINTS = src_points.length;
                for(var i = 0; i < NUM_POINTS; ++i)
                {
                    dst_points[i][0] -= dst_points[NUM_POINTS-1][0];
                    dst_points[i][1] -= dst_points[NUM_POINTS-1][1];
                    src_points[i][0] -= src_points[NUM_POINTS-1][0];
                    src_points[i][1] -= src_points[NUM_POINTS-1][1];
                }
            
                var perspective = wtransform.perspective();
                perspective.source_points(dst_points);
                perspective.target_points(src_points);

                _points = src_points.map(function(p){
                    return perspective(p);
                });

                // Center point no longer needed
                _points.pop();
            
                // Fit projection into the screen
                var minx = _points[0][0];
                var miny = _points[0][1];
                var maxx = minx;
                var maxy = miny;
            
                for(var i = 1; i < _points.length; ++i)
                {
                    if(_points[i][0] < minx) minx = _points[i][0];
                    if(_points[i][1] < miny) miny = _points[i][1];
                    if(_points[i][0] > maxx) maxx = _points[i][0];
                    if(_points[i][1] > maxy) maxy = _points[i][1];
                }
            
                var sx = Math.abs(maxx - minx)/_img_width;
                var sy = Math.abs(maxy - miny)/_img_height;
                var s = 1/Math.max(sx,sy);
            
                // Scale down points
                _points.forEach(p => {p[0] *= s; p[1] *= s});
            
                // Align projection
                var h_off = (_hkey > 0.0) ? (minx * s + half_w) : (maxx * s - half_w);
                var v_off = (_vkey > 0.0) ? (maxy * s - half_h) : (miny * s + half_h);

                _points.forEach(p => {
                    p[0] += (half_w - h_off);
                    p[1] += (half_h - v_off);
                });
            }

            function transform(point, idx)
            {
                return [_points[idx][0], _points[idx][1]];
            }

            transform.source_points = function(sp){
                _points = sp.map(function(d){return [d[0], d[1]]});
                _img_width = _points[3][0] - _points[0][0];
                _img_height = _points[3][1] - _points[0][1];
                update_points();
            }

            transform.horizontal = function(v){
                _hkey = v * (Math.PI/180);
                update_points();
            }

            transform.vertical = function(v){
                _vkey = v * (Math.PI/180);
                update_points();
            }

            return transform;
        }
    };
})();