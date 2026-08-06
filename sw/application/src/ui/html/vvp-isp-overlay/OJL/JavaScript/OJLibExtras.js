"use strict";

import { OJServerLink } from "./OJLExtras.js";
import { OJISPAWBDialog } from "./OJLExtras.js";

export function OJLibExtrasInit()
{
    OJServerLink.Get().AddDialog("ISPAWBDialog", (params, context) => new OJISPAWBDialog(params));
}