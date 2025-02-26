//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/csv_scanner/csv_file_scanner.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/operator/csv_scanner/csv_buffer_manager.hpp"
#include "duckdb/execution/operator/csv_scanner/scanner_boundary.hpp"
#include "duckdb/execution/operator/csv_scanner/csv_state_machine.hpp"
#include "duckdb/execution/operator/csv_scanner/csv_error.hpp"

namespace duckdb {
struct ReadCSVData;
//! Basic CSV Column Info
struct CSVColumnInfo {
	CSVColumnInfo(string &name_p, LogicalType &type_p) : name(name_p), type(type_p) {
	}
	string name;
	LogicalType type;
};

//! Basic CSV Schema
struct CSVColumnSchema {
	void Initialize(vector<string> &names, vector<LogicalType> &types, const string &file_path);
	bool Empty() const;
	bool SchemasMatch(string &error_message, vector<string> &names, vector<LogicalType> &types,
	                  const string &file_path);
	vector<CSVColumnInfo> columns;
	unordered_map<string, idx_t> name_idx_map;
	string file_path;
};

//! Struct holding information over a CSV File we will scan
class CSVFileScan {
public:
	//! Constructor for when a CSV File Scan is being constructed over information acquired during sniffing
	//! This means the options are alreadu set, and the buffer manager is already up and runinng.
	CSVFileScan(ClientContext &context, shared_ptr<CSVBufferManager> buffer_manager,
	            shared_ptr<CSVStateMachine> state_machine, const CSVReaderOptions &options,
	            const ReadCSVData &bind_data, const vector<column_t> &column_ids, CSVColumnSchema &file_schema);
	//! Constructor for new CSV Files, we must initialize the buffer manager and the state machine
	//! Path to this file
	CSVFileScan(ClientContext &context, const string &file_path, const CSVReaderOptions &options, const idx_t file_idx,
	            const ReadCSVData &bind_data, const vector<column_t> &column_ids, CSVColumnSchema &file_schema,
	            bool per_file_single_threaded);

	CSVFileScan(ClientContext &context, const string &file_name, CSVReaderOptions &options);

	void SetStart();
	const string &GetFileName();
	const vector<string> &GetNames();
	const vector<LogicalType> &GetTypes();
	void InitializeProjection();
	void Finish();

	//! Initialize the actual names and types to be scanned from the file
	void InitializeFileNamesTypes();
	const string file_path;
	//! File Index
	idx_t file_idx;
	//! Buffer Manager for the CSV File
	shared_ptr<CSVBufferManager> buffer_manager;
	//! State Machine for this file
	shared_ptr<CSVStateMachine> state_machine;
	//! How many bytes were read up to this point
	atomic<idx_t> bytes_read {0};
	//! Size of this file
	idx_t file_size;
	//! Line Info used in error messages
	shared_ptr<CSVErrorHandler> error_handler;
	//! Whether or not this is an on-disk file
	bool on_disk_file = true;

	vector<string> names;
	vector<LogicalType> types;
	MultiFileReaderData reader_data;

	vector<LogicalType> file_types;

	//! Variables to handle projection pushdown
	set<idx_t> projected_columns;
	std::vector<std::pair<idx_t, idx_t>> projection_ids;

	//! Options for this CSV Reader
	CSVReaderOptions options;

	CSVIterator start_iterator;
};
} // namespace duckdb
