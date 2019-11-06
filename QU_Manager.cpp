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
	Init_Result(res);
	res->col_num = nSelAttrs;
	int num = 1;
	RC rc;
	std::vector<std::pair<char*, QU_Records*>> tables;
	for (int i = 0;i < nRelations; ++i) {
		tables.push_back(std::make_pair(relations[i], new QU_Records));
		rc = GetRecordByTableName(relations[i], tables[i].second);
		CHECK(rc);
		num *= tables[i].second->nRecords;
	}
	std::vector<std::vector<AttrEntry>> tableAttributes;
	// Tranverse every record in the Cartesian product of all tables.
	for (int i = 0;i < num; ++i) {
		int x = i;
		std::vector<RM_Record*> record;
		// Take corresponding records of every table.
		for (size_t j = 0;j < tables.size(); ++j) {
			record.push_back(tables[j].second->records[x%tables[j].second->nRecords]);
			x /= tables[j].second->nRecords;
			std::vector<AttrEntry> attributes;
			int attrCount;
			if ((rc = ColumnEntryGet(tables[j].first, &attrCount, attributes))) {
				return rc;
			}
			tableAttributes.push_back(attributes);
		}
		bool checked = 1;
		// Go through all conditions.
		for (int j = 0;j < nConditions; ++j) {
			Condition* cond = &conditions[j];
			Value lhsValue, rhsValue;
			if (cond->bLhsIsAttr) {
				for (size_t k = 0;k < tables.size(); ++k) {
					if (cond->lhsAttr.relName == NULL || strcmp(cond->lhsAttr.relName, tables[k].first) == 0) {
						rc = GetRecordValue(record[k], cond->lhsAttr.attrName, tableAttributes[k], &lhsValue);
						if (rc == SUCCESS) {
							break;
						}
					}
				}
			} else {
				lhsValue = cond->lhsValue;
			}

			if (cond->bRhsIsAttr) {
				for (size_t k = 0;k < tables.size(); ++k) {
					if (cond->rhsAttr.relName == NULL || strcmp(cond->rhsAttr.relName, tables[k].first) == 0) {
						rc = GetRecordValue(record[k], cond->rhsAttr.attrName, tableAttributes[k], &rhsValue);
						if (rc == SUCCESS) {
							break;
						}
					}
				}
			} else {
				rhsValue = cond->rhsValue;
			}

			rc = check_cond(cond->op, lhsValue, rhsValue, checked);
			CHECK(rc);
			if (!checked) {
				checked = 0;
				break;
			}
		}
		if (checked) {
			// All conditions passed. Add to res.
			res->res[res->row_num] = new char*[nSelAttrs];
			for (int j = 0;j < nSelAttrs; ++j) {
				Value *value = new Value;
				value->data = NULL;
				for (size_t k = 0;k < tables.size(); ++k) {
					if (selAttrs[j]->relName == NULL) {
						rc = GetRecordValue(record[k], selAttrs[j]->attrName, tableAttributes[k], value);
						if (rc == SUCCESS) {
							break;
						}
					} else if (strcmp(tables[k].first, selAttrs[j]->relName) == 0) {
						rc = GetRecordValue(record[k], selAttrs[j]->attrName, tableAttributes[k], value);
						CHECK(rc);
					}
				}
				assert(value->data);
				if (!i) {
					res->type[j] = value->type;
					res->length[j] = GetValueLength(value);
					memcpy(res->fields[j], GetFullColumnName(selAttrs[j]), sizeof(char)*20);
				}
				res->res[res->row_num][j] = new char[20];
				memset(res->res[res->row_num][j], 0, 20);
				switch(value->type) {
					case chars:
						strcpy_s(res->res[res->row_num][j], 20, (char*)value->data);
						break;
					case ints:
						sprintf_s(res->res[res->row_num][j], 20, "%d", *(int*)value->data);
						break;
					case floats:
						sprintf_s(res->res[res->row_num][j], 20, "%f", *(float*)value->data);
						break;
					default:
						return INVALID_TYPES;
				}
			}
			res->row_num++;						
		}
	}
	return SUCCESS;
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

// Return "Table.Column" or "Column" if relName == NULL.
const char* GetFullColumnName(RelAttr* relAttr) {
	if (relAttr->relName == NULL) {
		return relAttr->attrName;
	}
	char* tmp = new char[strlen(relAttr->relName) + 1 + strlen(relAttr->attrName)];
	strcpy_s(tmp, strlen(relAttr->relName), relAttr->relName);
	tmp[strlen(relAttr->relName)] = '.';
	strcpy_s(tmp + strlen(relAttr->relName) + 1, strlen(relAttr->attrName), relAttr->attrName);
	return tmp;
}

/*
	Get all records for tableName
*/
RC GetRecordByTableName(char* tableName, QU_Records *records) {
	RM_FileHandle fileHandle;
	RC rc = RM_OpenFile(tableName, &fileHandle);
	CHECK(rc);
	RM_FileScan fileScan;
	rc = OpenScan(&fileScan, &fileHandle, 0, NULL);
	CHECK(rc);
	records->nRecords = 0;
	while(1) {
		RM_Record* record = new RM_Record;
		int recordSize;
		rc = GetRecordSize(tableName, &recordSize);
		CHECK(rc);
		record->pData = new char[recordSize];
		memset(record->pData, 0, recordSize);
		rc = GetNextRec(&fileScan, record);
		if (rc == RM_EOF) break;
		records->records[records->nRecords++] = record;
	}
	return SUCCESS;
}

/*
	Get value of attrName in rmRecord
*/
RC GetRecordValue(RM_Record* rmRecord, const char* attrName, const std::vector<AttrEntry>& attributes, Value* value) {
	bool found = 0;
	char* pData = rmRecord->pData;
	for (size_t i = 0; i < attributes.size(); i++) {
		if (attributes[i].attrName.compare(attrName) == 0) {
			value->type = attributes[i].attrType;
			value->data = pData + attributes[i].attrOffset;
			found = 1;
		}
	}
	if (found) {
		return SUCCESS;
	}
	return ATTR_NOT_EXIST;
}

template<typename T>
int QU_cmp(const T& x, const T& y) {
	if (x == y) return 0;
	if (x < y) return -1;
	return 1;
}

/*
	Set res to (lhsValue op rhsValue)
*/
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