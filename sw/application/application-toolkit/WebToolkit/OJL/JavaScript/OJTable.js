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
import { OJLib, UI } from "./OJL.js";

export class OJTable extends OJWindowElement
{
    constructor()
    {
        super();
        this._class_name = "OJTable";
        this._client_area.className = "oj_table";
        this._scale = 1;
        this._table_width = 900;
        this._table_height = 480;
        this._item_height = 28;
        this._rows_added = false;
        this._created = false;
        this._num_columns = 4;
        this._num_lines = 0;
        this._show_column_headers = true;
        this._callbacks = [];
        this._column_headers = [];
        this._rows = [];
        this._border = 4;
    };

    Destroy()
    {
        OJLib.DestroyArray(this._column_headers);
        OJLib.DestroyArray(this._rows);

        this._column_headers = null;
        this._rows = null;

        for (let i = 0; i < this._callbacks.length; i++)
            this._callbacks[i].Destroy();

        this._callbacks.length = 0;

        super.Destroy();
    }

    GetHeight()
    {
        let item_height = this._item_height * this._scale;
        let height = this._border + this._rows.length * (item_height + this._border);
        return height;
    }

    GetWidth()
    {
        let width = 0;
        for (let i = 0; i < this._column_headers.length; i++)
            width += this._column_headers[i]._width;

        return (width * this._scale) |0;
    }

    GetMaximumRowsForHeight(height)
    {
        let item_height = this._item_height * this._scale;
        let spare = height - this._border;
        let max_rows = (spare / (item_height + this._border)) | 0;
        return max_rows;
    }
    GetElementAtPosition(r, c)
    {
        let element = null;
        
        if ((r == 0) && this._show_column_headers)
        {
            element = this._column_headers[c]._element;
        }
        else
        {
            if (this._show_column_headers)
                r--;
            let row = this._rows[r];
            element = row._items[c]._element;
        }

        return element;
    }

    SetScale(scale)
    {
        this._scale = scale;
        let font = UI._large_font_name;
        this._border = 4;
        if (this._scale < 1)
        {
            font = _small_font_name;
            this._border = 2;
        }
        let n_rows = this._rows.length;
        if (n_rows == 0)
            return;

        let n_columns = 0;
        if (this._show_column_headers)
        {
            n_columns = this._column_headers.length;
            n_rows++;
        }
        else
            n_columns = this._rows[0]._items.length;

        // Reposition our babies
        let item_height = this._item_height;

        for (let r = 0; r < n_rows; r++)
        {
            let x = 0;

            for (let c = 0; c < n_columns; c++)
            {
                let column = this._column_headers[c];
                let item_width = column._width;

                let y = this._border + r * (item_height + this._border * 1);

                let item_element = this.GetElementAtPosition(r, c);
                if (item_element == null)
                    break;
                //item_element._x = x;
                let left = ((item_element._x + item_element._left_offset) * this._scale) | 0;
                let top = (y * this._scale) | 0;
                item_element.style.left = left + "px";
                item_element.style.top = top + "px";
                item_element.style.font = font;

                if ((r == 0) && this._show_column_headers)
                {
                    item_element.style.width = (((item_width * this._scale) | 0) - 5) + "px";
                }
                else
                {
                    item_element.style.width = (((item_width * this._scale) | 0) - 2) + "px";
                }

                this._client_area.appendChild(item_element);
                x += item_width; // + border * 2);
            }
        }
    }

    AddColumn(title, width, column, click_callback, object)
    {
        if (this._rows_added || this._created)
            return false;

        this._column_headers.push(new OJTableColumnHeader(title, width, column, click_callback, object));
        return true;
    }

    SetColumnWidth(column, width)
    {
        if (column < this._column_headers.length)
        {
            this._column_headers[column]._width = width;
        }
    }

    GetColumnWidth(column)
    {
        if (column < this._column_headers.length)
            return this._column_headers[column].GetWidth();
        else
            return null
    }

    GetNumColumns()
    {
        return this._column_headers.length;
    }
    AddRow()
    {
        if (this._created)
            return false;

        let n_rows = this._rows.length;
        this._rows.push(new OJTableRow(this, n_rows));

        return true;
    }

    GetRow(index)
    {
        if (index < this._rows.length)
            return this._rows[index];
        else
            return null;
    }

    ClearElements()
    {
        for (let i = 0; i < this._rows.length; i++)
        {
            let row = this._rows[i];
            for (let j = 0; j < row._items.length; j++)
            {
                let element = row._items[j]._element;
                element._user_data = null;
                element.innerHTML = "";
            }
        }
    }

    GetElement(row, column)
    {
        if (arguments.length == 0)
            return OJWindowElement.prototype.GetElement.call(this);

        let row_item = this._rows[row];
        if (row_item == null)
            return null;
        let item = row_item._items[column];
        if (item == null)
            return null;
        return item._element;
    }

    SetElementText(row, column, indentation, text, user_data)
    {
        let element = this.GetElement(row, column);
        if (element != null)
        {
            element.style.textIndent = (indentation * 45 + 5) + "px";
            element.innerHTML = text;
            element._user_data = user_data;
        }
    }

    SetElementColour(row, column, colour)
    {
        let element = this.GetElement(row, column);
        if (element != null)
            UI.SetStyleAttribute(element.style, "color", colour);
    }

    SetRowItems(row, indentation, text, control)
    {
        this.SetElementText(row, 0, 3, text);

        let control_parent_element = this.GetElement(row, 1);
        let control_element = control.GetElement();
        control_element.style.top = "1px";
        let background_colour = control_parent_element._private_background_colour;
        control_element.style.backgroundColor = background_colour;
        control_parent_element.appendChild(control_element);
    }

    ShowColumnHeaders(state)
    {
        this._show_column_headers = state;
    }

    SetColumnHilite(col, hilite)
    {
        if (this._show_column_headers)
        {
            this._column_headers[col].SetHilite(hilite);
            if (this._rows.length)
            {
                let element = this._column_headers[col]._element;
                element.className = hilite ? "header_row_hilite_class" : "header_row_dim_class";
            }
        }
    }

    GetValueColumnsWidth()
    {
        let n_columns = this._column_headers.length;
        let value_columns_width = 0;
        for (let c = 1; c < n_columns; c++)
        {
            let column = this._column_headers[c];
            value_columns_width += column._width;
        }

        return value_columns_width;
    }
    OnClick(event)
    {
        if (event._user_data._header != null)
        {
            event._user_data._header._click_callback.call(event._user_data._header._object, event);
        }
        else
        {
            if (this._cell_click_callback)
                this._cell_click_callback(event);
        }
    }

    Create(parent_element)
    {
        let n_columns = this._column_headers.length;
        let n_rows = this._rows.length;
        if (this._show_column_headers)
            n_rows++;

        let item_height = this._item_height * this._scale;
        this._border = 4;
        let font = UI._large_font_name;
        if (this._scale < 1)
        {
            font = _small_font_name;
            this._border = 2;
        }

        for (let r = 0; r < n_rows; r++)
        {
            let even_steven = ((r % 2) == 0);
            let x = 0;

            for (let c = 0; c < n_columns; c++)
            {
                let column = this._column_headers[c];
                let item_width = (column._width * this._scale) | 0;
                let item_element = document.createElement("div");
                item_element._left_offset = 0;

                if ((r == 0) && this._show_column_headers)
                    item_element.className = "header_row_class";
                else
                    item_element.className = even_steven ? "even_row_class" : "odd_row_class";

                item_element._private_background_colour = even_steven ? "#181818" : "#121212";

                let y = this._border + r * (item_height + this._border * 1);

                item_element._x = x;
                item_element.style.left = (item_element._x + item_element._left_offset) + "px";
                item_element.style.top = y + "px";
                item_element.style.font = font;

                if ((r == 0) && this._show_column_headers)
                {
                    item_element.style.width = item_width + "px";
                    this._column_headers[c]._element = item_element;
                    item_element.innerHTML = column._title;
                    if (this._column_headers[c]._click_callback && this._column_headers[c]._hilite)
                    {
                        item_element.className = "header_row_hilite_class";
                    }
                    if (this._column_headers[c]._click_callback)
                    {
                        let button_callbacks = OJLib.RegisterButton(this, item_element, { _header: this._column_headers[c], _row: r, _column: c });
                        this._callbacks.push(button_callbacks);
                    }
                }
                else
                {
                    let row_index = r;
                    if (this._show_column_headers)
                        row_index--;
                    let row = this._rows[row_index];
                    row.SetElement(c, item_element);
                    if (this._cell_click_callback)
                    {
                        let button_callbacks = OJLib.RegisterButton(this, item_element, { _header: null, _row: r, _column: c });
                        this._callbacks.push(button_callbacks);
                        item_element.row = r;
                        item_element.column = c;
                    }
                }

                this._client_area.appendChild(item_element);

                x += item_width; //(item_width + border * 2);
            }
        }

        if (parent_element != null)
            parent_element.appendChild(this._client_area);

        this._created = true;
    }
}

export class OJTableColumnHeader
{
    constructor(title, width, column, click_callback, object)
    {
        this._title  = title;
        this._width  = width;
        this._hilite = false;
        this._column = column;
        if (click_callback)
        {
            this._object = object;
            this._click_callback = click_callback;
        }
    }

    SetHilite(hilite)
    {
        this._hilite = hilite;
    }

    GetWidth()
    {
        return this._width;
    }
}

export class OJTableRow
{
    constructor(table, row_i, user_data)
    {
        this._row_i = row_i;
        this._table = table;
        this._items = [];
        this._user_data = user_data;

        for (let i = 0; i < table.GetNumColumns() ; i++)
            this._items.push(new OJTableItem(row_i, i, table.GetColumnWidth(i)));
    }

    Destroy()
    {
        this._table = 0;
    }

    SetElement(column, element)
    {
        this._items[column].SetElement(element);
    }

    SetUserData(user_data)
    {
        this._user_data = user_data;
    }

    GetUserData()
    {
        return this._user_data;
    }
}

export class OJTableItem
{
    constructor(row, column, width)
    {
        this._element = null;
        this._row = row;
        this._column = column;
        this._width = width;
    }

    SetElement(element)
    {
        this._element = element;
        this._element.style.width = this._width + "px";
    }
}
