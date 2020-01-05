#include "StdAfx.h"
#include "QU_Manager.h"

void Init_Result(SelResult* res) {
	memset(res, 0, sizeof(SelResult));
	res->next_res = NULL;
}

void Destory_Result(SelResult* res) {
	for (int i = 0; i < res->row_num; i++) {
		for (int j = 0; j < res->col_num; j++) {
			delete[] res->res[i][j];
		}
		delete[] res->res[i];
	}
	if (res->next_res != NULL) {
		Destory_Result(res->next_res);
	}
}

RC Query(char* sql, SelResult* res) {
	sqlstr* tmp = get_sqlstr();
	parse(sql, tmp);
	if (tmp->flag != 1) {
		return NOT_SELECT;
	}
	return Select(
		tmp->sstr.sel.nSelAttrs,
		tmp->sstr.sel.selAttrs,
		tmp->sstr.sel.nRelations,
		tmp->sstr.sel.relations,
		tmp->sstr.sel.nConditions,
		tmp->sstr.sel.conditions,
		res);
}

#define CHECK(rc) if (rc != SUCCESS) {return rc;}

RC Select(int nSelAttrs, RelAttr** selAttrs, int nRelations, char** relations, int nConditions, Condition* conditions, SelResult* res) {
	if (res == NULL) {
		return INVALID_VALUES;
	}
	// Reverse these arrays due to unknown bug of flex.
	std::reverse(selAttrs, selAttrs + nSelAttrs);
	std::reverse(relations, relations + nRelations);
	std::reverse(conditions, conditions + nConditions);
	Init_Result(res);
	RC rc;
	std::vector<std::pair<std::string, QU_Records>> tableRecords; // Store all Records for every table.
	std::vector<std::vector<AttrEntry>> tableAttributes; // Store a vector of AttrEntry for every table.
	std::map<std::pair<std::string, std::string>, AttrEntry> tableAttrMap; // Map <tableName, columnName> to the AttrEntry of this colunm.
	std::map<std::string, int> tableOrder; // Map table name to it's order.
	std::vector<Condition> conds;
	for (int i = 0;i < nRelations; ++i) {
		tableRecords.push_back(std::make_pair(relations[i], QU_Records()));
		tableOrder[relations[i]] = i;
		// Get column information.
		std::vector<AttrEntry> attributes;
		int attrCount;
		rc = ColumnEntryGet(relations[i], &attrCount, attributes);
		CHECK(rc);
		for (auto attr : attributes) {
			tableAttrMap[std::make_pair(tableRecords[i].first, attr.attrName)] = attr;
		}
		// Fill null relName in Conditions.
		for (int j = 0;j < nConditions; ++j) {
			auto cond = conditions[j];
			AttrEntry tmp;
			if (cond.bLhsIsAttr && cond.lhsAttr.relName == NULL && FindAttr(tableAttrMap, tableRecords[i].first, cond.lhsAttr.attrName, tmp) == SUCCESS) {
				conditions[j].lhsAttr.relName = relations[i];
			}
			if (cond.bRhsIsAttr && cond.rhsAttr.relName == NULL && FindAttr(tableAttrMap, tableRecords[i].first, cond.rhsAttr.attrName, tmp) == SUCCESS) {
				conditions[j].rhsAttr.relName = relations[i];
			}
		}
		tableAttributes.push_back(attributes);
	}
	// Modify every condition so that the table that has the less order always appears at left, if both sides are attrs;
	// if there's only one side of attr, then make sure that attr appears at right;
	// if both sides are only values, check and ignore it in future evaluation.
	// For cond.value.data, fill it with attr's table's order.
	for (int j = 0;j < nConditions; ++j) {
		auto cond = conditions[j];
		int mask = (cond.bLhsIsAttr << 1) | cond.bRhsIsAttr;
		if (mask == 0b11) { // attr op attr
			int x = tableOrder[cond.lhsAttr.relName];
			int y = tableOrder[cond.rhsAttr.relName];
			conditions[j].lhsValue.data = (void*)x;
			conditions[j].rhsValue.data = (void*)y;
			if (x > y) ReverseCond(conditions[j]);
		} else if (mask == 0b10) { // attr op value
			int x = tableOrder[cond.lhsAttr.relName];
			conditions[j].lhsValue.data = (void*)x;
			ReverseCond(conditions[j]);
		} else if (mask == 0b00) { // value op value
			bool tmp = 0;
			rc = check_cond(cond.op, cond.lhsValue, cond.rhsValue, tmp);
			CHECK(rc);
			if (!tmp) {
				return SUCCESS;
			}
			continue;
		} else { // value op attr
			int y = tableOrder[cond.rhsAttr.relName];
			conditions[j].rhsValue.data = (void*)y;
		}
		conds.push_back(conditions[j]);
	}
	// Sort conds basing on the order of their right attr's table.
	std::sort(conds.begin(), conds.end(), condCmp);
	int num = 1;
	for (int i = 0;i < nRelations; ++i) {
		// Get all records for every relation.
		QU_Records allRecords;
		rc = GetRecordByTableName(relations[i], allRecords);
		CHECK(rc);
		// Filter records by conditions only relating to table[i].
		rc = FilterRecordByCondition(tableAttrMap, relations[i], conds, allRecords, tableRecords[i].second);
		CHECK(rc);
		// Count the size of output rows.
		num *= tableRecords[i].second.size();
	}
	bool star = 0;
	if (nSelAttrs == 1 && strcmp(selAttrs[0]->attrName, "*") == 0) {
		// For "SELECT *", refill selAttrs with all entries.
		star = 1;
		nSelAttrs = tableAttrMap.size();
		selAttrs = new RelAttr* [nSelAttrs];
		nSelAttrs = 0;
		for (size_t i = 0; i < tableRecords.size(); ++i) {
			for (size_t j = 0; j < tableAttributes[i].size(); ++j) {
				selAttrs[nSelAttrs] = new RelAttr;

				selAttrs[nSelAttrs]->relName = new char[tableRecords[i].first.length() + 1];
				strcpy(selAttrs[nSelAttrs]->relName, tableRecords[i].first.c_str());
				selAttrs[nSelAttrs]->relName[tableRecords[i].first.length()] = '\0';

				selAttrs[nSelAttrs]->attrName = new char[tableAttributes[i][j].attrName.length() + 1];
				strcpy(selAttrs[nSelAttrs]->attrName, tableAttributes[i][j].attrName.c_str());
				selAttrs[nSelAttrs]->attrName[tableAttributes[i][j].attrName.length()] = '\0';

				nSelAttrs++;
			}
		}
	}
	res->col_num = nSelAttrs;
	// Perform the select.
	std::vector<SelStatus> status;
	std::vector<RM_Record> records;
	std::vector<int> recordSizes;
	for (size_t i = 0;i < tableRecords.size(); ++i) {
		int j;
		rc = GetRecordSize((char*)tableRecords[i].first.c_str(), &j);
		CHECK(rc);
		recordSizes.push_back(j);
	}
	status.push_back(SelStatus(-1));
	records.push_back(RM_Record());
	while (!status.empty()) {
		records.pop_back();
		// Find next record for the last column.
		bool columnEof = 0;
		if (status.back().is_index) {
			RM_Record rec;
			rec.pData = new char[recordSizes[status.size()-1]];
			rc = status.back().index.Next(&rec);
			if (rc == SUCCESS) {
				records.push_back(rec);
			} else
			if (rc == IX_EOF) {
				columnEof = 1;
			} else {
				// Other errors.
			}
		} else {
			if (status.back().i + 1 == tableRecords[status.size()-1].second.size()) {
				columnEof = 1;
			} else {
				auto rec = tableRecords[status.size()-1].second[++status.back().i];
				records.push_back(*rec);
			}
		}
		if (columnEof) {
			// Go back one table and find it's next record.
			if (status.back().is_index) {
				status.back().index.Close();
			}
			status.pop_back();
			continue;
		} else {
			// Apply conds on this record.
			bool checked = 1;
			for (auto cond : conds) {
				if ((int)cond.rhsValue.data == status.size()-1) {
					Value lValue, rValue;
					if (cond.bLhsIsAttr) {
						GetRecordValue(
							&records[(int)cond.lhsValue.data],
							cond.lhsAttr.attrName,
							tableAttrMap[std::make_pair(cond.lhsAttr.relName, cond.lhsAttr.attrName)],
							lValue);
					} else {
						lValue = cond.lhsValue;
					}
					GetRecordValue(
						&records[status.size()-1],
						cond.rhsAttr.attrName,
						tableAttrMap[std::make_pair(cond.rhsAttr.relName, cond.rhsAttr.attrName)],
						rValue);
					check_cond(cond.op, lValue, rValue, checked);
					if (!checked) {
						break;
					}
				}
			}
			if (!checked) {
				// Get next valid record.
				continue;
			}
		}
		if (status.size() < tableRecords.size()) {
			// Tranverse the next table.
			char* tableName = (char*)tableRecords[status.size()].first.c_str(), *columnName;
			bool found = 0;
			for (auto cond : conds) {
				if ((int)cond.rhsValue.data == status.size()) {
					columnName = cond.rhsAttr.attrName;
					auto attr = tableAttrMap[std::make_pair(tableName, columnName)];
					if (attr.ix_flag) {
						Value lValue;
						if (cond.bLhsIsAttr) {
							GetRecordValue(
								&records[(int)cond.lhsValue.data],
								cond.lhsAttr.attrName,
								tableAttrMap[std::make_pair(cond.lhsAttr.relName, cond.lhsAttr.attrName)],
								lValue);
						} else {
							lValue = cond.lhsValue;
						}
						int is_success = 0;
						SelStatus selStatus(tableName, columnName, cond.op, (char*)lValue.data, is_success);
						if (is_success) {
							status.push_back(selStatus);
							records.push_back(RM_Record());
							found = 1;
							break;
						}
					}
				}
 			}
			if (found) {
				continue;
			} else {
				status.push_back(SelStatus(-1));
				records.push_back(RM_Record());
				continue;
			}
		} else {
			// Add to res.
			if (res->row_num == 100) {
				res->next_res = new SelResult;
				res = res->next_res;
				Init_Result(res);
				res->col_num = nSelAttrs;
			}
			res->res[res->row_num] = new char*[nSelAttrs];
			for (int j = 0;j < nSelAttrs; ++j) {
				Value value;
				value.data = NULL;
				for (size_t k = 0;k < tableRecords.size(); ++k) {
					AttrEntry tmp;
					if (selAttrs[j]->relName == NULL) {
						rc = FindAttr(tableAttrMap, tableRecords[k].first, selAttrs[j]->attrName, tmp);
						if (rc == ATTR_NOT_EXIST) {
							continue;
						}
						rc = GetRecordValue(&records[k], selAttrs[j]->attrName, tmp, value);
						if (rc == SUCCESS) {
							break;
						}
					} else if (strcmp(tableRecords[k].first.c_str(), selAttrs[j]->relName) == 0) {
						rc = FindAttr(tableAttrMap, tableRecords[k].first, selAttrs[j]->attrName, tmp);
						CHECK(rc);
						rc = GetRecordValue(&records[k], selAttrs[j]->attrName, tmp, value);
						CHECK(rc);
					}
				}
				assert(value.data);
				if (!res->row_num) {
					res->type[j] = value.type;
					res->length[j] = GetValueLength(&value);
					memcpy(res->fields[j], GetFullColumnName(selAttrs[j]), sizeof(char)*20);
				}
				res->res[res->row_num][j] = new char[20];
				memset(res->res[res->row_num][j], 0, 20);
				switch(value.type) {
					case chars:
						strcpy_s(res->res[res->row_num][j], 20, (char*)value.data);
						break;
					case ints:
						sprintf_s(res->res[res->row_num][j], 20, "%d", *(int*)value.data);
						break;
					case floats:
						sprintf_s(res->res[res->row_num][j], 20, "%f", *(float*)value.data);
						break;
					default:
						return INVALID_TYPES;
				}
			}
			res->row_num++;						
			continue;
		}
	}
	return SUCCESS;
}

void ReverseOp(CompOp& op) {
	switch (op) {
	case LEqual: // <=
		op = GEqual;
		break;
	case GEqual: // >=
		op = LEqual;
		break;
	case LessT: // <
		op = GreatT;
		break;
	case GreatT: // >
		op = LessT;
		break;
	}
}

void ReverseCond(Condition& cond) {
	std::swap(cond.bLhsIsAttr, cond.bRhsIsAttr);
	std::swap(cond.lhsAttr, cond.rhsAttr);
	std::swap(cond.lhsValue, cond.rhsValue);
	ReverseOp(cond.op);
}

bool condCmp(const Condition& x, const Condition& y) {
	return x.rhsValue.data < y.rhsValue.data;
}

RC FindAttr(std::map<std::pair<std::string, std::string>, AttrEntry>& tableAttrMap, const std::string& tableName, const std::string& columnName, AttrEntry& attr) {
	auto p = std::make_pair(tableName, columnName);
	if (tableAttrMap.count(p)) {
		attr = tableAttrMap[p];
		return SUCCESS;
	}
	return ATTR_NOT_EXIST;
}

// Get length of value.
int GetValueLength(const Value* v) {
	switch(v->type) {
		case chars:
			return strlen((char*)v->data);
		case ints:
			return 4;
		case floats:
			return 4;
		default:
			return 0;
	}
}

RC GetValueForCond(
	bool isAttr, RelAttr attr, Value cond_value, std::map<std::pair<std::string, std::string>, AttrEntry>& tableAttrMap, RM_Record* record, Value& value) {
	RC rc;
	if (isAttr) {
		AttrEntry tmp;
		rc = FindAttr(tableAttrMap, attr.relName, attr.attrName, tmp);
		CHECK(rc);
		rc = GetRecordValue(record, attr.attrName, tmp, value);
		CHECK(rc);
	} else {
		value = cond_value;
	}
	return SUCCESS;
}

/* Return "Table.Column" or "Column" if relName == NULL. */
const char* GetFullColumnName(RelAttr* relAttr) {
	if (relAttr->relName == NULL) {
		return relAttr->attrName;
	}
	int length = strlen(relAttr->relName) + 1 + strlen(relAttr->attrName) + 1;
	char* tmp = new char[length];
	strcpy_s(tmp, length, relAttr->relName);
	tmp[strlen(relAttr->relName)] = '.';
	strcpy_s(tmp + strlen(relAttr->relName) + 1, length - strlen(relAttr->relName) - 1, relAttr->attrName);
	tmp [strlen(relAttr->relName) + 1 + strlen(relAttr->attrName)] = '\0';
	return tmp;
}

/* Get all records for tableName. */
RC GetRecordByTableName(char* tableName, QU_Records& records) {
	RM_FileHandle fileHandle;
	RC rc = RM_OpenFile(tableName, &fileHandle);
	CHECK(rc);
	RM_FileScan fileScan;
	rc = OpenScan(&fileScan, &fileHandle, 0, NULL);
	CHECK(rc);
	records.clear();
	while(1) {
		RM_Record* record = new RM_Record;
		int recordSize;
		rc = GetRecordSize(tableName, &recordSize);
		CHECK(rc);
		record->pData = new char[recordSize];
		memset(record->pData, 0, recordSize);
		rc = GetNextRec(&fileScan, record);
		if (rc == RM_EOF) break;
		records.push_back(record);
	}
	return SUCCESS;
}

RC FilterRecordByCondition(
	std::map<std::pair<std::string, std::string>, AttrEntry>& tableAttrMap,
	const char* relation, std::vector<Condition> conditions, QU_Records& in, QU_Records& out) {
	RC rc;
	out.clear();
	for (auto rec : in) {
		bool checked = 1;
		for (auto cond : conditions) {
			if (cond.bLhsIsAttr && strcmp(cond.lhsAttr.relName, relation)) {
				continue;
			}
			if (cond.bRhsIsAttr && strcmp(cond.rhsAttr.relName, relation)) {
				continue;
			}
			Value lhsValue, rhsValue;
			rc = GetValueForCond(
				cond.bLhsIsAttr, cond.lhsAttr, cond.lhsValue, tableAttrMap, rec, lhsValue);
			CHECK(rc);
			rc = GetValueForCond(
				cond.bRhsIsAttr, cond.rhsAttr, cond.rhsValue, tableAttrMap, rec, rhsValue);
			CHECK(rc);
			rc = check_cond(cond.op, lhsValue, rhsValue, checked);
			CHECK(rc);
			if (!checked) {
				break;
			}
		}
		if (checked) {
			out.push_back(rec);
		}
	}
	return SUCCESS;
}

/* Get value of attrName in rmRecord. */
RC GetRecordValue(RM_Record* rmRecord, const char* attrName, const AttrEntry& attr, Value& value) {
	char* pData = rmRecord->pData;
	value.type = attr.attrType;
	value.data = pData + attr.attrOffset;
	return SUCCESS;
}

template<typename T>
int QU_cmp(const T& x, const T& y) {
	if (x == y) return 0;
	if (x < y) return -1;
	return 1;
}

/* Set res to (lhsValue op rhsValue) */
RC check_cond(const CompOp& op, const Value& lhsValue, const Value& rhsValue, bool& res) {
	assert(lhsValue.type == rhsValue.type);
	int cmp;
	switch (lhsValue.type) {
		case chars:
		cmp = strcmp((char*)lhsValue.data, (char*)rhsValue.data);
		break;
		case ints:
		cmp = QU_cmp(*(int*)lhsValue.data, *(int*)rhsValue.data);
		break;
		case floats:
		cmp = QU_cmp(*(float*)lhsValue.data, *(float*)rhsValue.data);
		break;
		default:
		return INVALID_TYPES;
	}
	return check_cmp(op, cmp, res);
}

RC check_cmp(const CompOp& op, const int& cmp, bool& res) {
	switch (op) {
		case EQual:
		res = cmp == 0;
		break;
		case LEqual:
		res = cmp <= 0;
		break;
		case NEqual:
		res = cmp != 0;
		break;
		case LessT:
		res = cmp < 0;
		break;
		case GEqual:
		res = cmp >= 0;
		break;
		case GreatT:
		res = cmp > 0;
		break;
		case NO_OP:
		return INVALID_CONDITIONS;
		break;
	}
	return SUCCESS;
}