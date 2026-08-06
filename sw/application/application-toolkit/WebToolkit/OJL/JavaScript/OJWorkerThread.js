/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

"use strict";

import { OJLib } from "./OJL.js";

export class OJWorkerThread
{
    constructor(owner, javascript_filename)
    {
        // We can't derive from Worker, so have a Worker member instead
        this._worker = new Worker(javascript_filename);
        this._worker._oj_worker_thread = this;
        this._class_name = "OJWorkerThread";
        this._owner = owner;
        this._javascript_filename = javascript_filename;

        this._worker.onerror = function(event)
        {
            OJLib.Trace("OJWorkerThread: " + event.message + " (" + event.filename + ":" + event.lineno + ")");
        };

        this._worker_thread_listener = function(event)
        {
            if (this._oj_worker_thread != null)
                this._oj_worker_thread.OnWorkerThread(event);
        };

        this._worker.addEventListener("message", this._worker_thread_listener, false);
    }

    Destroy()
    {
        this._owner = null;
        this._worker.postMessage({ _shutdown: true });
    }

    DestroyComplete()
    {
        //this._worker.removeEventListener("message", this._worker_thread_listener, false);
        //this._worker_thread_listener = null;
        this._worker._oj_worker_thread = null;
        //this._worker.terminate();

        //threads_to_terminate.push(this._worker);

        //this._worker = null;

        //if (flush_threads_timeout_id != null)
        //	clearTimeout(flush_threads_timeout_id);

        //flush_threads_timeout_id = setTimeout(OnFlushThreads, 200);
    }

    OnWorkerThread(event)
    {
        // Track shutdown response
        if (event.data._shutdown_response != null)
        {
            this.DestroyComplete();
        }
        else if (event.data._web_socket_closed != null)
        {
            OJLib.Trace("OJWorkerThread: WebSocket closed: " + event.data._reason);
        }
        else if (event.data._trace_message != null)
        {
            OJLib.Trace(event.data._trace_message);
        }
        else
        {
            if (this._owner != null)
                this._owner.OnWorkerThread(event);
        }
    }

    PostMessage(message)
    {
        this._worker.postMessage(message);
    }
}

let threads_to_terminate = [];
let flush_threads_timeout_id = null;

function OnFlushThreads()
{
    if (threads_to_terminate.length > 0)
    {
        threads_to_terminate[0].terminate();
        threads_to_terminate.splice(0, 1);
    }

    if (threads_to_terminate.length > 0)
    {
        if (flush_threads_timeout_id != null)
            clearTimeout(flush_threads_timeout_id);

        flush_threads_timeout_id = setTimeout(OnFlushThreads, 200);
    }
}
