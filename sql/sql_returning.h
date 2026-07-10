/* Copyright (c) 2015, 2026, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef SQL_RETURNING_INCLUDED
#define SQL_RETURNING_INCLUDED

#include "mem_root_deque.h"
#include "sql/protocol.h"      // Protocol
#include "sql/query_result.h"  // Query_result_send

class Item;
class THD;

/**
  Streams a DML statement's RETURNING clause back to the client as a result
  set. Wraps a Query_result_send: send column metadata once, then one row per
  affected row, then an EOF. Used by INSERT/REPLACE/UPDATE/DELETE ... RETURNING.
*/
class Returning_sender {
 public:
  /// Create the result sink and send the result-set metadata (column defs).
  bool begin(THD *thd, const mem_root_deque<Item *> &fields) {
    m_result = new (thd->mem_root) Query_result_send();
    if (m_result == nullptr) return true;
    return m_result->send_result_set_metadata(
        thd, fields, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF);
  }
  /// Evaluate and send one RETURNING row (from the current record buffer).
  bool send_row(THD *thd, const mem_root_deque<Item *> &fields) {
    return m_result->send_data(thd, fields);
  }
  /// Finish the result set with an EOF packet.
  bool end(THD *thd) { return m_result->send_eof(thd); }

 private:
  Query_result_send *m_result{nullptr};
};

#endif /* SQL_RETURNING_INCLUDED */
