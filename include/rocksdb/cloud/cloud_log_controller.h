//  Copyright (c) 2016-present, Rockset, Inc.  All rights reserved.
//
#pragma once
#include <atomic>
#include <thread>

#include "rocksdb/cloud/cloud_options.h"
#include "rocksdb/env.h"
#include "rocksdb/status.h"

namespace ROCKSDB_NAMESPACE {
class CloudEnv;
class Logger;
struct CloudEnvOptions;

// Creates a new file, appends data to a file or delete an existing file via
// logging into a cloud stream (such as Kinesis).
//
class CloudLogWritableFile : public WritableFile {
 public:
  CloudLogWritableFile(CloudEnv* env, const std::string& fname,
                       const EnvOptions& options);
  virtual ~CloudLogWritableFile();

  virtual Status Flush() {
    assert(status_.ok());
    return status_;
  }

  virtual Status Sync() {
    assert(status_.ok());
    return status_;
  }

  virtual Status status() { return status_; }

  // Appends data to the file. If the file doesn't exist, it'll get created.
  virtual Status Append(const Slice& data) = 0;

  // Closes a file by writing an EOF marker to the Cloud stream.
  virtual Status Close() = 0;

  // Delete a file by logging a delete operation to the Cloud stream.
  virtual Status LogDelete() = 0;

 protected:
  CloudEnv* env_;
  const std::string fname_;
  Status status_;
};

struct CloudLogControllerOptions : public CloudOptions {
  // request timeout for requests from the cloud storage. A value of 0
  // means the default timeout assigned by the underlying cloud storage.
  uint64_t request_timeout_ms = 600000;
  
  // connection timeout for requests from the cloud storage. A value of 0
  // means the default timeout assigned by the underlying cloud storage.
  uint64_t connect_timeout_ms = 30000;
};

class CloudLogController {
 public:
  static const std::string kControllerOpts /*= "cloudlog" */;
  static const std::string kLogKafka /* = "kafka" */;
  static const std::string kLogKinesis /* = "kinesis" */;

  CloudLogController(const CloudLogControllerOptions& options);
  virtual ~CloudLogController();

  static Status CreateLogController(
      const std::string& name, std::unique_ptr<CloudLogController>* result);
  static Status CreateLogController(
      const std::string& name, const CloudLogControllerOptions& options,
      std::unique_ptr<CloudLogController>* result);

  // Create a stream to store all log files.
  virtual Status CreateStream(const std::string& topic) = 0;

  // Waits for stream to be ready (blocking).
  virtual Status WaitForStreamReady(const std::string& topic) = 0;

  // Continuously tail the cloud log stream and apply changes to
  // the local file system (blocking).
  virtual Status TailStream() = 0;

  // Creates a new cloud log writable file.
  virtual CloudLogWritableFile* CreateWritableFile(
      const std::string& fname, const EnvOptions& options) = 0;

  // Returns name of the cloud log type (Kinesis, etc.).
  virtual const char* Name() const { return "cloudlog"; }

  // print out all options to the log
  virtual void Dump(Logger* log) const;

  // Directory where files are cached locally.
  virtual const std::string& GetCacheDir() const = 0;
  virtual Status const status() const = 0;

  virtual Status StartTailingStream(const std::string& topic) = 0;
  virtual void StopTailingStream() = 0;
  virtual Status GetFileModificationTime(const std::string& fname,
                                         uint64_t* time) = 0;
  virtual Status NewSequentialFile(const std::string& fname,
                                   std::unique_ptr<SequentialFile>* result,
                                   const EnvOptions& options) = 0;
  virtual Status NewRandomAccessFile(const std::string& fname,
                                     std::unique_ptr<RandomAccessFile>* result,
                                     const EnvOptions& options) = 0;
  virtual Status FileExists(const std::string& fname) = 0;
  virtual Status GetFileSize(const std::string& logical_fname,
                             uint64_t* size) = 0;
  // Prepares/Initializes the log controller for the input cloud environment

  virtual Status Prepare(CloudEnv* env) = 0;
  template <typename T>
  const T* GetOptions(const std::string& name) const {
    return reinterpret_cast<const T*>(GetOptionsPtr(name));
  }
  template <typename T>
  T* GetOptions(const std::string& name) {
    return reinterpret_cast<T*>(const_cast<void*>(GetOptionsPtr(name)));
  }

  template <typename T>
  const T* CastAs(const std::string& name) const {
    const auto c = FindInstance(name);
    return static_cast<const T*>(c);
  }

  template <typename T>
  T* CastAs(const std::string& name) {
    auto c = const_cast<CloudLogController*>(FindInstance(name));
    return static_cast<T*>(c);
  }

 protected:
  CloudLogControllerOptions options_;
  virtual const CloudLogController* FindInstance(const std::string& name) const;
  virtual const void* GetOptionsPtr(const std::string& name) const;
};

}  // namespace ROCKSDB_NAMESPACE