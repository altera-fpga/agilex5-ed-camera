/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

function OJTiffHeader()
{
	// Base class constructor
	this._buffer = new Uint8Array(8);
	var i = 0;

	// Little endian
	this._buffer[i++] = 0x49;
	this._buffer[i++] = 0x49;

	// Version
	this._buffer[i++] = 0x2a;
	this._buffer[i++] = 0x00;
	
	// IFDOffset
	this._buffer[i++] = 0x08;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
}

OJTiffHeader.prototype.GetBuffer = function()
{
	return this._buffer;
};

function OJTiffImageFileDirectory(tags)
{
	this._tags = tags;
	var num_tags = this._tags.length;
	var ifd_size = 2 + (num_tags * 12) + 4;

	this._buffer = new Uint8Array(ifd_size);

	// Number of tags
	var i = 0;
	this._buffer[i++] = (num_tags & 0x00ff);
	this._buffer[i++] = (num_tags & 0xff00) >> 8;
};

OJTiffImageFileDirectory.prototype.CopyTagValues = function()
{
	var num_tags = this._tags.length;
	var i = 2;

	// Copy out tag data
	for (var tag_index = 0; tag_index < num_tags; tag_index++)
	{
		var tiff_tag = this._tags[tag_index];
		var tag_data = tiff_tag.GetBuffer();
		for (var n = 0; n < 12; n++)
			this._buffer[i++] = tag_data[n];
	}

	// Next IFD offset
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
};

OJTiffImageFileDirectory.prototype.GetBuffer = function()
{
	return this._buffer;
};

var TIFF_TAG_ID =
{ 
	NewSubfileType: 254,
	SubfileType: 255,
	ImageWidth: 256,
	ImageHeight: 257,
	BitsPerSample: 258,
	Compression: 259,
	PhotometricInterpretation: 262,
	StripOffsets: 273,
	SamplesPerPixel: 277,
	RowsPerStrip: 278,
	StripByteCounts: 279,
	XResolution: 282,
	YResolution: 283,
	PlanarConfiguration: 284,
	ResolutionUnit: 296,
	Software: 305,
	DateTime: 306,
	Artist: 315
};

function OJTiffTag(tag_id, value, opts)
{
	this._buffer = new Uint8Array(12);
	var i = 0;
	this._buffer[i++] = (tag_id & 0x00ff);
	this._buffer[i++] = (tag_id & 0xff00) >> 8;

	if ((opts != null) && (opts._type == "rational"))
	{
		// Data type rational
		this._buffer[i++] = 0x05;
		this._buffer[i++] = 0x00;
	}
	else if (value == null)
	{
		for (i = 2; i < 12; i++)
			this._buffer[i] = 0x00;
	}
	else if (typeof value == "string")
	{

	}
	else if (value <= 0xffff)
	{
		// Data type = 16 bit unsigned
		this._buffer[i++] = 0x03;
		this._buffer[i++] = 0x00;

		// Num items
		this._buffer[i++] = 0x01;
		this._buffer[i++] = 0x00;
		this._buffer[i++] = 0x00;
		this._buffer[i++] = 0x00;

		this._buffer[i++] = (value & 0x00ff);
		this._buffer[i++] = (value & 0xff00) >> 8;
		this._buffer[i++] = 0x00;
		this._buffer[i++] = 0x00;
	}
	else
	{
		// Data type = 32 bit unsigned
		this._buffer[i++] = 0x04;
		this._buffer[i++] = 0x00;

		// Num items
		this._buffer[i++] = 0x01;
		this._buffer[i++] = 0x00;
		this._buffer[i++] = 0x00;
		this._buffer[i++] = 0x00;

		this._buffer[i++] = (value & 0x000000ff);
		this._buffer[i++] = (value & 0x0000ff00) >> 8;
		this._buffer[i++] = (value & 0x00ff0000) >> 16;
		this._buffer[i++] = (value & 0xff000000) >> 24;
	}
}

OJTiffTag.prototype.GetBuffer = function (event)
{
	return this._buffer;
}

OJTiffTag.prototype.SetValue = function(value)
{
	var i = 2;

	// Data type = 32 bit unsigned
	this._buffer[i++] = 0x04;
	this._buffer[i++] = 0x00;

	// Num items
	this._buffer[i++] = 0x01;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;

	this._buffer[i++] = (value & 0x000000ff);
	this._buffer[i++] = (value & 0x0000ff00) >> 8;
	this._buffer[i++] = (value & 0x00ff0000) >> 16;
	this._buffer[i++] = (value & 0xff000000) >> 24;
};

OJTiffTag.prototype.AddValues = function (offset, num_dwords)
{
	var i = 2;

	// Data type = 32 bit unsigned
	this._buffer[i++] = 0x04;
	this._buffer[i++] = 0x00;

	this._buffer[i++] = num_dwords;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;

	this._buffer[i++] = (offset & 0x000000ff);
	this._buffer[i++] = (offset & 0x0000ff00) >> 8;
	this._buffer[i++] = (offset & 0x00ff0000) >> 16;
	this._buffer[i++] = (offset & 0xff000000) >> 24;

	return offset + (num_dwords * 4);
};

OJTiffTag.prototype.AddString = function(offset, the_string)
{
	var i = 2;

	// Data type = null terminated string
	this._buffer[i++] = 0x02;
	this._buffer[i++] = 0x00;

	this._buffer[i++] = the_string.length + 1;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;
	this._buffer[i++] = 0x00;

	this._buffer[i++] = (offset & 0x000000ff);
	this._buffer[i++] = (offset & 0x0000ff00) >> 8;
	this._buffer[i++] = (offset & 0x00ff0000) >> 16;
	this._buffer[i++] = (offset & 0xff000000) >> 24;

	return offset + the_string.length + 1;
};

//////////////////

export function OJTiffFile(width, height, rgb_image_data)
{
	this._width = width;
	this._height = height;
	this._rgb_image_data = rgb_image_data;
}

OJTiffFile.prototype.ConcatenateBuffers = function(buffers)
{
	var new_length = 0;
	for (var i = 0; i < buffers.length; i++)
		new_length += buffers[i].length;

    var combined_buffer = new (buffers[0].constructor)(new_length);

	var position = 0;
	for (var i = 0; i < buffers.length; i++)
	{
		combined_buffer.set(buffers[i], position);
		position += buffers[i].length;
	}

    return combined_buffer;
};

OJTiffFile.prototype.Save = function(filename)
{
	if (this._rgb_image_data == null)
		return;
		
	var current_date = new Date(); 
	var date_and_time_str = "" 
					+ this.PadNum(current_date.getFullYear(), 4) + ":"  
					+ this.PadNum(current_date.getMonth()+1, 2) + ":" 
					+ this.PadNum(current_date.getDate(), 2) + " "
					+ this.PadNum(current_date.getHours(), 2) + ":"  
					+ this.PadNum(current_date.getMinutes(), 2) + ":" 
					+ this.PadNum(current_date.getSeconds(), 2);	
	var blocks = [];
	var tags = [];
	var strips_per_image = 1;
	var samples_per_pixel = 3;
	tags.push(new OJTiffTag(TIFF_TAG_ID.NewSubfileType, 0));
	tags.push(new OJTiffTag(TIFF_TAG_ID.ImageWidth, this._width));
	tags.push(new OJTiffTag(TIFF_TAG_ID.ImageHeight, this._height));
	var bits_per_sample_tag = new OJTiffTag(TIFF_TAG_ID.BitsPerSample, 0);
	tags.push(bits_per_sample_tag);
	tags.push(new OJTiffTag(TIFF_TAG_ID.Compression, 1));
	tags.push(new OJTiffTag(TIFF_TAG_ID.PhotometricInterpretation, 2));
	var strip_offsets_tag = new OJTiffTag(TIFF_TAG_ID.StripOffsets, 0);
	tags.push(strip_offsets_tag);
	tags.push(new OJTiffTag(TIFF_TAG_ID.SamplesPerPixel, samples_per_pixel));
	tags.push(new OJTiffTag(TIFF_TAG_ID.RowsPerStrip, this._height));
	tags.push(new OJTiffTag(TIFF_TAG_ID.StripByteCounts, this._width * this._height * samples_per_pixel * 2));
	var x_resolution = new OJTiffTag(TIFF_TAG_ID.XResolution, 0,  { _type: "rational" });
	tags.push(x_resolution);
	var y_resolution = new OJTiffTag(TIFF_TAG_ID.YResolution, 0, { _type: "rational" });
	tags.push(y_resolution);
	tags.push(new OJTiffTag(TIFF_TAG_ID.PlanarConfiguration, 1));
	tags.push(new OJTiffTag(TIFF_TAG_ID.ResolutionUnit, 2));
	var software_tag = new OJTiffTag(TIFF_TAG_ID.Software);
	tags.push(software_tag);
	var date_tag = new OJTiffTag(TIFF_TAG_ID.DateTime);
	tags.push(date_tag);
	var artist_tag = new OJTiffTag(TIFF_TAG_ID.Artist);
	tags.push(artist_tag);

	var header = new OJTiffHeader();
	blocks.push(header.GetBuffer());

	var image_file_header = new OJTiffImageFileDirectory(tags);
	blocks.push(image_file_header.GetBuffer());

	var data_values_offset = 0;
	for (var i = 0; i < blocks.length; i++)
		data_values_offset += blocks[i].length;

	var start_data_values_offset = data_values_offset;
	var bits_per_sample_offset = data_values_offset;
	data_values_offset = bits_per_sample_tag.AddValues(data_values_offset, 3);

	var strip_offsets_offset = data_values_offset;
	data_values_offset = strip_offsets_tag.AddValues(data_values_offset, 1);

	var x_resolution_offset = data_values_offset;
	data_values_offset = x_resolution.AddValues(data_values_offset, 2);

	var y_resolution_offset = data_values_offset;
	data_values_offset = y_resolution.AddValues(data_values_offset, 2);

	var software_str_offset = data_values_offset;
	var software_tag_str = "Intel Application Toolkit";
	data_values_offset = software_tag.AddString(data_values_offset, software_tag_str);

	var date_str_offset = data_values_offset;
	data_values_offset = date_tag.AddString(data_values_offset, date_and_time_str);

	var artist_str_offset = data_values_offset;
	var artist_tag_str = "Intel OJL";
	data_values_offset = artist_tag.AddString(data_values_offset, artist_tag_str);

	// Allocate values array
	var data_values_size = data_values_offset - start_data_values_offset;
	var data_values = new Uint8Array(data_values_size);

	// Write values
	var strip_start = data_values_offset;
	strip_offsets_tag.SetValue(strip_start);
	this.WriteUint32s(data_values, bits_per_sample_offset - start_data_values_offset, [16, 16, 16]);
	this.WriteUint32s(data_values, strip_offsets_offset - start_data_values_offset, [strip_start]);
	this.WriteUint32s(data_values, x_resolution_offset - start_data_values_offset, [600, 1]);
	this.WriteUint32s(data_values, y_resolution_offset - start_data_values_offset, [600, 1]);
	this.WriteString(data_values, software_str_offset - start_data_values_offset, software_tag_str);
	this.WriteString(data_values, date_str_offset - start_data_values_offset, date_and_time_str);
	this.WriteString(data_values, artist_str_offset - start_data_values_offset, artist_tag_str);

	blocks.push(data_values);
	blocks.push(this._rgb_image_data);

	image_file_header.CopyTagValues();

	// rgb_image_data
	var combined_buffer = this.ConcatenateBuffers(blocks);

	var blob = new Blob([combined_buffer], { type: "application/octet-stream" });
	var file_url = URL.createObjectURL(blob);
	var file_link = document.createElement("a");
	file_link.href = file_url;
	file_link.download = filename;
	file_link.click();
};
	
OJTiffFile.prototype.PadNum = function(num, n_chars)
{
	var str = "" + num;
	return str.padStart(n_chars, "0");
};

OJTiffFile.prototype.WriteUint32s = function(data_values, offset, values)
{
	for (var i = 0; i < values.length; i++)
	{
		var value = values[i];
		data_values[offset++] = (value & 0x000000ff);
		data_values[offset++] = (value & 0x0000ff00) >> 8;
		data_values[offset++] = (value & 0x00ff0000) >> 16;
		data_values[offset++] = (value & 0xff000000) >> 24;
	}
};

OJTiffFile.prototype.WriteString = function(data_values, offset, tag_string)
{
	for (var i = 0; i < tag_string.length; i++)
		data_values[offset++] = tag_string.charCodeAt(i);

	data_values[offset++] = 0;
};

