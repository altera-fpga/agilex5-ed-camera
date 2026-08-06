/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJWindowElement } from "./OJL.js";
import { wsvg, wmath, wtransform } from "./OJL.js";

//////////////////////////////////////////////////////////////////////////////////////

const TransformType = 
{
    FIXED: 0,
    PERSPECTIVE: 1,
    ARBITRARY: 2,
    FISHEYE: 3
};

let FisheyeMapping = 
{
    PANORAMA:   0,
    EQUIRECT:   1,
    UNITY:      2
};

const to_radians = (degrees) => degrees * (Math.PI / 180);


export class OJWarpWireframe extends OJWindowElement
{
    constructor()
    {
        // Base class constructor
        super();
        
        this._handle_radius = 8;
        
        // Work out widget width and height using specified width as guide
        // and the aspect ratio to calculate the height

        // Set by Resize
        let width = 320;
        let height = 180;
        this._svg_width = width;
        let img_width = this._svg_width - 2 * this._handle_radius;

        let square_size = img_width / 16;

        let img_height = square_size * 9;
        this._svg_height = img_height + 2 * this._handle_radius;
        
        this._img_width = img_width;
        this._img_height = img_height;
        this._show_transform_mesh = true;
        this._warning_state = false;

        this._sourcePoints = [[0, 0], [img_width, 0], [0, img_height], [img_width, img_height]];

        this._perspective = wtransform.perspective();
        this._perspective.source_points(this._sourcePoints);

        this._affine = wtransform.affine();
        this._affine.origin([img_width/2, img_height/2]);

        this._keystone = wtransform.keystone();
        this._keystone.source_points(this._sourcePoints);

        this._radial = wtransform.radial();
        this._radial.resolution(img_width, img_height);
        this._radial.focus([img_width/2, img_height/2]);

        this._svg = wsvg.create("svg");
        this._svg.setAttribute("width", this._svg_width);
        this._svg.setAttribute("height", this._svg_height);
        this._svg.style.backgroundColor = "#d8d8de";
        this._svg.style.border = "#b7b7b7";
        this._svg.style.borderWidth = "1px";
        this._svg.style.borderStyle = "solid";

        this._helper_grid = [];
        this._full_helper_grid = [];
        this._show_guide = true;


        this._view = wsvg.create("g"); 
        this._img = wsvg.create("g");
        this._view.appendChild(this._img);

        this.SetViewScale(1.0);

        // Default arbitray knots
        this._normalized_grid_points = [[]];
        this._corner_points = [];
        this.SetArbitraryKnotsNum(2);
        this._vlines = [];
        this._hlines = [];
        this._current_transform = TransformType.FIXED;
        
        this._grid = [];

        this._fisheye_mapping = FisheyeMapping.UNITY;

        this._fisheye_lens_fov = to_radians(180.0);
        this._ftp_radius = 1.0;

        // Panorama mapping parameters
        this._ftp_latitude_min = 0.0;
        this._ftp_latitude_max = to_radians(90.0);
        this._ftp_longitude = 0.0;
        this._ftp_horizontal_fov = to_radians(90.0);

        // Equirectangular mapping parameters
        this._equirect_vfov = to_radians(180.0);
        this._equirect_hfov = to_radians(360.0);

        this._svg.appendChild(this._view);
        this.GetElement().appendChild(this._svg);
    }

    Resize(x, y, width, height)
    {
        // Call base
        let size_changed = super.Resize(x, y, width, height);

        if (size_changed)
        {
            // 1 pixel border on either side
            this._svg_width = width - 2;
            let img_width = this._svg_width - 2 * this._handle_radius;
        
            let square_size = img_width / 16;
        
            let img_height = square_size * 9;
            this._svg_height = img_height + 2 * this._handle_radius - 2;
            
            this._img_width = img_width;
            this._img_height = img_height;
            this._sourcePoints = [[0, 0], [img_width, 0], [0, img_height], [img_width, img_height]];
            this._perspective.source_points(this._sourcePoints);
            this._affine.origin([img_width/2, img_height/2]);
            this._keystone.source_points(this._sourcePoints);
            this._radial.resolution(img_width, img_height);
            this._radial.focus([img_width/2, img_height/2]);
            this._svg.setAttribute("width", this._svg_width);
            this._svg.setAttribute("height", this._svg_height);
        
            // Helper grid
            let x_range = wsvg.range(0, img_width + 1, square_size);
            x_range = x_range.map(function(x) { return [[x, 0], [x, img_height]]; });

            let y_range = wsvg.range(0, img_height + 1, square_size);
            y_range = y_range.map(function(y) { return [[0, y], [img_width, y]]; });

            for (let path of this._full_helper_grid)
            {
                if (this._view.contains(path))
                    this._view.removeChild(path);
            }        

            this._full_helper_grid.length = 0;
            this._helper_grid.length = 0;

            let first_child = this._view.firstChild;
            for (let i = 0; i < x_range.length; i++)
            {
                let guide_line = x_range[i];
                let path = wsvg.create("path");
                path.setAttribute("d", wsvg.path_data(guide_line));
                if ((i == 0) || (i == (x_range.length - 1)))
                {
                    path.setAttribute("stroke", "#303030");
                    this._view.insertBefore(path, first_child);
                }
                else 
                {
                    path.setAttribute("stroke", "#a0a0a0");
                    this._helper_grid.push(path);
                }

                this._full_helper_grid.push(path);
            }
            
            for (let i = 0; i < y_range.length; i++)
            {
                let guide_line = y_range[i];
                let path = wsvg.create("path");
                path.setAttribute("d", wsvg.path_data(guide_line));
                if ((i == 0) || (i == (y_range.length - 1)))
                {
                    path.setAttribute("stroke", "#303030");
                    this._view.insertBefore(path, first_child);
                }
                else 
                {
                    path.setAttribute("stroke", "#a0a0a0");
                    this._helper_grid.push(path);
                }
        
                this._full_helper_grid.push(path);
            }        

            if (this._show_guide)
                this.ShowGuide(true);

            if ((this._current_transform == TransformType.FIXED) ||
                (this._current_transform == TransformType.PERSPECTIVE))
            {
                this.GeneratePerspectiveGrid();

                if (this._current_transform == TransformType.FIXED)
                    this.TransformFixed();
                else
                {        
                    // Restore the corner points
                    for (let i = 0; i < this._corner_points.length; i++)
                    {
                        let pos = this._corner_points[i];
                        this._perspective.set_point(i, [pos[0] * this._img_width, pos[1] * this._img_height]);
                    } 
                    
                    this.TransformPerspective();
                }
            }
            else if (this._current_transform == TransformType.ARBITRARY)
            {

                this.GenerateArbitraryGrid();

                for (let i = 0; i < this._normalized_grid_points.length; i++)
                {
                    for (let j = 0; j < this._normalized_grid_points[i].length; j++)
                    {
                        let pos = this._normalized_grid_points[i][j];
                        let dstPos = this._gridDstPoints[i][j];
                        dstPos[0] = pos[0] * this._img_width;
                        dstPos[1] = pos[1] * this._img_height;
                    }
                }

                this.TransformedArbitrary();
            }
            else if (this._current_transform == TransformType.FISHEYE)
            {
                this.DrawFisheyeDiagram();
            }

            this.SetViewScale(this._view_scale);
        }

        return size_changed;
    }

    SetViewScale(view_scale)
    {
        this._view_scale = view_scale;
        let dx = (((1 - this._view_scale) * this._svg_width) / 2);
        let dy = (((1 - this._view_scale) * this._svg_height) / 2);
        dx += this._handle_radius;
        dy += this._handle_radius;

        // Center lines in middle of pixel
        dx -= 0.5;
        dy -= 0.5;
        this._view.setAttribute("transform", "translate(" + dx + " " + dy + ") scale(" + this._view_scale + " " + this._view_scale + ")");
    }

    ShowGuide(state)
    {
        this._show_guide = state;
        if (state)
        {
            let first_child = this._view.firstChild;
            for (let path of this._helper_grid)
            {
                if (!this._view.contains(path))
                    this._view.insertBefore(path, first_child);
            }
        }
        else
        {
            for (let path of this._helper_grid)
            {
                if (this._view.contains(path))
                    this._view.removeChild(path);
            }
        }
    }

    Reset()
    {
        this._perspective = wtransform.perspective();
        this._perspective.source_points(this._sourcePoints);

        this._affine = wtransform.affine();
        this._affine.origin([this._img_width/2, this._img_height/2]);

        this._keystone = wtransform.keystone();
        this._keystone.source_points(this._sourcePoints);

        this._radial = wtransform.radial();
        this._radial.resolution(this._img_width, this._img_height);
        this._radial.focus([this._img_width/2, this._img_height/2]);
    }

    GenerateGrid(width, height, hstep, vstep)
    {
        let vlines = [];
        let hlines = [];
        let grid = [];

        let sub_steps = 2;

        let xStepsBig = wsvg.range(0, width + 1, hstep);
        let yStepsBig = wsvg.range(0, height + 1, vstep);
        let xStepsSmall = wsvg.range(0, width + 1, hstep/sub_steps);
        let yStepsSmall = wsvg.range(0, height + 1, vstep/sub_steps);

        this._gridSrcPoints = yStepsSmall.map(y => { return xStepsSmall.map(x => {return [x,y]})});
        this._gridDstPoints = yStepsSmall.map(y => { return xStepsSmall.map(x => {return [x,y]})});

        let num_rows = this._normalized_grid_points.length;
        let num_columns = this._normalized_grid_points[0].length;

        if ((this._gridSrcPoints.length != num_rows) ||
            (this._gridSrcPoints[0].length != num_columns))
        {
            this._normalized_grid_points.length = 0;
            this._normalized_grid_points = new Array(this._gridSrcPoints.length);
            for (let y = 0; y < this._gridSrcPoints.length; y++)
            {
                this._normalized_grid_points[y] = new Array(this._gridSrcPoints[0].length);
        
                for (let x = 0; x < this._gridSrcPoints[0].length; x++)
                    this._normalized_grid_points[y][x] = [0, 0];
            }
        }

        // Vertical lines
        for (let i = 0; i < xStepsSmall.length; i += sub_steps)
        {
            let line = [];
            for (let j = 0; j < yStepsSmall.length; ++j)
            {
                let point = this._gridDstPoints[j][i];
                line.push(point);
            }
            vlines.push(line);
        }

        // Horizontal lines
        for (let i = 0; i < yStepsSmall.length; i += sub_steps)
        {
            let line = [];
            for (let j = 0; j < xStepsSmall.length; ++j)
            {
                let point = this._gridDstPoints[i][j];
                line.push(point);
            }
            hlines.push(line);
        }

        let colour = this._warning_state ? "#f04040" : "#0068b5";
        let dash_array = this._warning_state ? "4 2" : null;

        let center_pixel_translate = "translate(-0.5 -0.5)";

        vlines.forEach(line => {
            let path = wsvg.create("path");
            path._d = line;
            path.setAttribute("d", wsvg.path_data(line));
            path.setAttribute("stroke", colour);
            path.setAttribute("stroke-dasharray", dash_array);
            path.setAttribute("fill", "none");
            path.setAttribute("stroke-width", "2px");
            path.setAttribute("transform", center_pixel_translate);
            grid.push(path);		
        });

        hlines.forEach(line => {
            let path = wsvg.create("path");
            path._d = line;
            path.setAttribute("d", wsvg.path_data(line));
            path.setAttribute("stroke", colour);
            path.setAttribute("stroke-dasharray", dash_array);
            path.setAttribute("fill", "none");
            path.setAttribute("stroke-width", "2px");
            path.setAttribute("transform", center_pixel_translate);
            grid.push(path);		
        });

        this._vlines = vlines;
        this._hlines = hlines;

        return grid;
    }

    AddGrid()
    {
        for (let path of this._grid)
        {
            if (!this._img.contains(path))
                this._img.appendChild(path);
        }
    }

    RemoveGrid()
    {
        for (let path of this._grid)
        {
            if (this._img.contains(path))
                this._img.removeChild(path);
        }
    }

    ShowTransformMesh(state)
    {
        this._show_transform_mesh = state;
        if (this._show_transform_mesh)
            this.AddGrid();
        else
            this.RemoveGrid();
    }

    GeneratePerspectiveGrid()
    {
        this.RemoveGrid();
        let hstep = this._img_width / 16;
        let vstep = this._img_height / 9;
        this._grid = this.GenerateGrid(this._img_width, this._img_height, hstep, vstep);

        if (this._show_transform_mesh)
            this.AddGrid();
    }

    SetArbitraryKnotsNum(num_knots)
    {
        this._num_knots = num_knots;
    }

    GenerateArbitraryGrid()
    {
        this.RemoveGrid();
        let hstep = this._img_width / (this._num_knots - 1);
        let vstep = this._img_height / (this._num_knots - 1);
        this._grid = this.GenerateGrid(this._img_width, this._img_height, hstep/4, vstep/4);

        if (this._show_transform_mesh)
            this.AddGrid();
    }

    DrawFisheyePanoramaDiagram(cx, cy, radius)
    {
        const startAngle = this._ftp_longitude;
        const endAngle = startAngle + this._ftp_horizontal_fov;

        const angleDiff = Math.abs(endAngle - startAngle) % (Math.PI * 2);
        const isFullCircle = angleDiff < 0.001;

        const radius1 = Math.sin(this._ftp_latitude_min) * this._ftp_radius * radius;
        const radius2 = Math.sin(this._ftp_latitude_max) * this._ftp_radius * radius;

        // Negation here to convert from clockwise to anti-clockwise angles,
        // as SVG arc draws in anti-clockwise direction for positive angles
        const startRad = -(startAngle);
        const endRad = -(endAngle);

        // Calculate start and end points of the arc1
        const x1 = cx + radius1 * Math.cos(startRad);
        const y1 = cy + radius1 * Math.sin(startRad);
        const x2 = cx + radius1 * Math.cos(endRad);
        const y2 = cy + radius1 * Math.sin(endRad);

        // Calculate start and end points of the arc2
        const x3 = cx + radius2 * Math.cos(startRad);
        const y3 = cy + radius2 * Math.sin(startRad);
        const x4 = cx + radius2 * Math.cos(endRad);
        const y4 = cy + radius2 * Math.sin(endRad);

        let largeArcFlag = (endAngle - startAngle) <= Math.PI ? 0 : 1;

        let pathData = [];

        if (isFullCircle)
        {
            const x1_ = cx + radius1 * Math.cos(startRad - Math.PI);
            const y1_ = cy + radius1 * Math.sin(startRad - Math.PI);
            const x3_ = cx + radius2 * Math.cos(startRad - Math.PI);
            const y3_ = cy + radius2 * Math.sin(startRad - Math.PI);

            pathData = [
                `M ${x1} ${y1}`,
                `L ${x3} ${y3}`,
                `A ${radius2} ${radius2} 0 ${largeArcFlag} 1 ${x3_} ${y3_}`,
                `A ${radius2} ${radius2} 0 ${largeArcFlag} 1 ${x4} ${y4}`,
                `L ${x2} ${y2}`,
                `A ${radius1} ${radius1} 0 ${largeArcFlag} 0 ${x1_} ${y1_}`,
                `A ${radius1} ${radius1} 0 ${largeArcFlag} 0 ${x1} ${y1}`,
                'Z'
            ].join(' ');
        }
        else
        {
            pathData = [
                `M ${x1} ${y1}`,
                `L ${x3} ${y3}`,
                `A ${radius2} ${radius2} 0 ${largeArcFlag} 0 ${x4} ${y4}`,
                `L ${x2} ${y2}`,
                `A ${radius1} ${radius1} 0 ${largeArcFlag} 1 ${x1} ${y1}`,
                'Z'
            ].join(' ');
        }

        let sector = wsvg.create("path");
        sector.setAttribute("d", pathData);

        return sector;
    }

    DrawFisheyeEquirectangularDiagram(cx, cy, radius)
    {
        const sphere_to_fisheye = (latitude, longitude) => {
            // Point on sphere
            const sph_x = Math.cos(latitude) * Math.sin(longitude);
            const sph_y = Math.sin(latitude);
            const sph_z = Math.cos(latitude) * Math.cos(longitude);

            // Fisheye angle and radius
            const fe_theta = Math.acos(sph_z);
            const fe_phi = Math.atan2(sph_y, sph_x);

            // Use lens original fov to compute radius, which is more accurate for extreme points
            const fe_r = radius * (this._ftp_radius * fe_theta / (this._fisheye_lens_fov / 2.0));
            
            // Pixel in fisheye space
            const fe_x = cx + fe_r * Math.cos(fe_phi);
            const fe_y = cy + fe_r * Math.sin(fe_phi);                

            return [fe_x, fe_y];
        };

        const latitude1 = -(this._equirect_vfov / 2.0);
        const latitude2 = latitude1 + this._equirect_vfov;
        const longitude1 = -(this._equirect_hfov / 2.0);
        const longitude2 = longitude1 + this._equirect_hfov;

        let points = [];
        const numSegments = 16;
        for (let i = 0; i <= numSegments; i++) {
            const t = i / numSegments;
            const latitude = latitude1 + t * (latitude2 - latitude1);
            const longitude = longitude1;
            const point = sphere_to_fisheye(latitude, longitude);
            points.push(point);
        }

        for (var i = 0; i <= numSegments; i++) {
            const t = i / numSegments;
            const latitude = latitude2;
            const longitude = longitude1 + t * (longitude2 - longitude1);
            const point = sphere_to_fisheye(latitude, longitude);
            points.push(point);
        }

        for (var i = 0; i <= numSegments; i++) {
            const t = i / numSegments;
            const latitude = latitude2 - t * (latitude2 - latitude1);
            const longitude = longitude2;
            const point = sphere_to_fisheye(latitude, longitude);
            points.push(point);
        }

        for (var i = 0; i <= numSegments; i++) {
            const t = i / numSegments;
            const latitude = latitude1;
            const longitude = longitude2 - t * (longitude2 - longitude1);
            const point = sphere_to_fisheye(latitude, longitude);
            points.push(point);
        }            

        let pathData = wsvg.path_data(points);
        let sector = wsvg.create("path");
        sector.setAttribute("d", pathData);

        return sector;
    }

    DrawFisheyeDiagram()
    {
        this.RemoveGrid();

        let cx = this._img_width / 2;
        let cy = this._img_height / 2;
        let radius = Math.min(this._img_width, this._img_height) / 2;

        let colour = this._warning_state ? "#f04040" : "#0068b5";

        // Lens outline
        let circle = wsvg.create("circle");

        circle.setAttribute("cx", cx);
        circle.setAttribute("cy", cy);
        circle.setAttribute("r", this._ftp_radius * radius);
        circle.setAttribute("fill", "none");
        circle.setAttribute("stroke", colour);
        circle.setAttribute("stroke-width", "2px");

        this._grid.push(circle);

        if (this._show_transform_mesh)
        {    
            this._img.appendChild(circle);
        }        

        // Outline of the projection being displayed
        let diagram = null;

        if(this._fisheye_mapping == FisheyeMapping.PANORAMA)
        {
            diagram = this.DrawFisheyePanoramaDiagram(cx, cy, radius);
        }
        else if(this._fisheye_mapping == FisheyeMapping.EQUIRECT)
        {
            diagram = this.DrawFisheyeEquirectangularDiagram(cx, cy, radius);
        }

        if(diagram)
        {
            diagram.setAttribute("fill", "#e0e0e8");
            diagram.setAttribute("stroke", colour);
            diagram.setAttribute("stroke-width", "2px");

            this._grid.push(diagram);

            if (this._show_transform_mesh)
            {
                this._img.appendChild(diagram);
            }
        }

        this._current_transform = TransformType.FISHEYE;        
    }

    SetCornerPoint(idx, pos)
    {
        this._perspective.set_point(idx, [pos[0] * this._img_width, pos[1] * this._img_height]);
        this._corner_points[idx] = pos;
        this.TransformPerspective();
    }

    SetRotation(v)
    {
        this._affine.set_rotate(v);
        this.TransformFixed();
    }

    SetZoom(v)
    {
        this._affine.set_zoom(v);
        this.TransformFixed();
    }

    SetHorizonalOffset(v)
    {
        this._affine.set_h_offset(v * this._img_width);
        this.TransformFixed();
    }

    SetVerticalOffset(v)
    {
        this._affine.set_v_offset(v * this._img_height);
        this.TransformFixed();
    }

    SetHorizontalKeystone(v)
    {
        this._keystone.horizontal(v);
        this.TransformFixed();
    }

    SetVerticalKeystone(v)
    {
        this._keystone.vertical(v);
        this.TransformFixed();
    }

    SetHorizontalMirror(v)
    {
        this._affine.set_h_mirror(v);
        this.TransformFixed();
    }

    SetVerticalMirror(v)
    {
        this._affine.set_v_mirror(v);
        this.TransformFixed();
    }

    SetRadial(v, corner_radial)
    {
        this._radial.strength(v);

        if (corner_radial)
            this.TransformPerspective();
        else    
            this.TransformFixed();
    }

    SetArbitraryPoint(idx, pos)
    {
        try{
            let i = ((idx / this._num_knots) >> 0) * 4 * 2;	// segments * substeps
            let j = (idx % this._num_knots) * 4 * 2;
            let dstPos = this._gridDstPoints[i][j];
            this._normalized_grid_points[i][j] = pos;
            dstPos[0] = pos[0] * this._img_width;
            dstPos[1] = pos[1] * this._img_height;
            this.TransformedArbitrary();
        }
        catch(error){
            console.log(error);
        }	
    }

    SetFisheyeLensFov(v)
    {
        this._fisheye_lens_fov = to_radians(v);
        this.DrawFisheyeDiagram();
    }

    SetFtpRadius(v)
    {
        this._ftp_radius = v;
        this.DrawFisheyeDiagram();
    }    

    SetFtpLatitudeMin(v)
    {
        this._ftp_latitude_min = to_radians(v);
        this.DrawFisheyeDiagram();
    }

    SetFtpLatitudeMax(v)
    {
        this._ftp_latitude_max = to_radians(v);
        this.DrawFisheyeDiagram();
    }

    SetFtpLongitude(v)
    {
        this._ftp_longitude = to_radians(v);
        this.DrawFisheyeDiagram();
    }

    SetFtpHorizontalFov(v)
    {
        this._ftp_horizontal_fov = to_radians(v);
        this.DrawFisheyeDiagram();
    }

    SetEquirectVfov(v)
    {
        this._equirect_vfov = to_radians(v);
        this.DrawFisheyeDiagram();
    }

    SetEquirectHfov(v)
    {
        this._equirect_hfov = to_radians(v);
        this.DrawFisheyeDiagram();
    }

    TransformFixed()
    {
        if (this._sourcePoints)
        {
            let targetPoints = this._sourcePoints.map((d,idx) => 
            {
                let p = this._keystone(d,idx);
                return this._affine(p,idx);
            })

            this._perspective.target_points(targetPoints);

            this.TransformPerspective();
        }
        this._current_transform = TransformType.FIXED;
    }

    TransformPerspective()
    {
        let owner = this;

        if (this._gridSrcPoints)
        {
            this._gridSrcPoints.forEach((line, i) => 
            {
                line.forEach((psrc, j) => 
                {
                    let _p = owner._radial(psrc);
                    _p = owner._perspective(_p);
                    let pdst = owner._gridDstPoints[i][j];
                    pdst[0] = _p[0];
                    pdst[1] = _p[1];
                });
            });

            for (let path of this._grid)
                path.setAttribute("d", wsvg.path_data(path._d));
        }

        this._current_transform = TransformType.PERSPECTIVE;
    }

    TransformedArbitrary()
    {
        this._hlines.forEach(function(d){wmath.apply_spline(d, 8)});
        this._vlines.forEach(function(d){wmath.apply_spline(d, 8)});
        this._hlines.forEach(function(d){wmath.apply_spline(d, 8)});

        for (let path of this._grid)
            path.setAttribute("d", wsvg.path_data(path._d));

        this._current_transform = TransformType.ARBITRARY;
    }

    ShowWarning(show)
    {
        this._warning_state = show;

        let colour = this._warning_state ? "#f04040" : "#0068b5";
        let dash_array = this._warning_state ? "4 2" : null;
        for (let path of this._grid)
        {
            path.setAttribute("stroke", colour);
            path.setAttribute("stroke-dasharray", dash_array);
        }
    }

    SetFisheyeMapping(v)
    {
        this._fisheye_mapping = v;

        if(this._current_transform == TransformType.FISHEYE)
            this.DrawFisheyeDiagram();
    }
}