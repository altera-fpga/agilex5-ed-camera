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
// class OJGeoLocator
///////////////////////////////////////////////////////////////////////////////
var _geocoder = null;
function OJGeoLocator()
{
	_geocoder = new google.maps.Geocoder();
	var geolocation = navigator.geolocation;
	geolocation.getCurrentPosition(this.SuccessCallback, this.ErrorCallback,
													  { maximumAge: 60000 });
}

OJGeoLocator.prototype.SuccessCallback = function(position)
{
	alert("Success: latitude = " + position.coords.latitude + ", longitude = " +
				position.coords.longitude);

	var lat_long = new google.maps.LatLng(position.coords.latitude,
                                          position.coords.longitude);

	var request = { location: lat_long };
	_geocoder.geocode(request,
				function(response, status)
				{
					var place = response[0];
					var address = place.formatted_address;

					alert("Your address is: " + address);
				});
};
