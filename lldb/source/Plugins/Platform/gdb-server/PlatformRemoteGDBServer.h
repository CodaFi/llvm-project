//===-- PlatformRemoteGDBServer.h ----------------------------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_PLATFORM_GDB_SERVER_PLATFORMREMOTEGDBSERVER_H
#define LLDB_SOURCE_PLUGINS_PLATFORM_GDB_SERVER_PLATFORMREMOTEGDBSERVER_H

#include <string>

#include "Plugins/Process/Utility/GDBRemoteSignals.h"
#include "Plugins/Process/gdb-remote/GDBRemoteCommunicationClient.h"
#include "Plugins/Process/gdb-remote/GDBRemoteCommunicationReplayServer.h"
#include "lldb/Target/Platform.h"

namespace lldb_private {
namespace platform_gdb_server {

class PlatformRemoteGDBServer : public Platform, private UserIDResolver {
public:
  static void Initialize();

  static void Terminate();

  static lldb::PlatformSP CreateInstance(bool force, const ArchSpec *arch);

  static ConstString GetPluginNameStatic();

  static const char *GetDescriptionStatic();

  PlatformRemoteGDBServer();

  ~PlatformRemoteGDBServer() override;

  // lldb_private::PluginInterface functions
  ConstString GetPluginName() override { return GetPluginNameStatic(); }

  uint32_t GetPluginVersion() override { return 1; }

  // lldb_private::Platform functions
  Status
  ResolveExecutable(const ModuleSpec &module_spec, lldb::ModuleSP &module_sp,
                    const FileSpecList *module_search_paths_ptr) override;

  bool GetModuleSpec(const FileSpec &module_file_spec, const ArchSpec &arch,
                     ModuleSpec &module_spec) override;

  const char *GetDescription() override;

  Status GetFileWithUUID(const FileSpec &platform_file, const UUID *uuid_ptr,
                         FileSpec &local_file) override;

  bool GetProcessInfo(lldb::pid_t pid, ProcessInstanceInfo &proc_info) override;

  uint32_t FindProcesses(const ProcessInstanceInfoMatch &match_info,
                         ProcessInstanceInfoList &process_infos) override;

  Status LaunchProcess(ProcessLaunchInfo &launch_info) override;

  Status KillProcess(const lldb::pid_t pid) override;

  lldb::ProcessSP DebugProcess(ProcessLaunchInfo &launch_info,
                               Debugger &debugger,
                               Target *target, // Can be NULL, if NULL create a
                                               // new target, else use existing
                                               // one
                               Status &error) override;

  lldb::ProcessSP Attach(ProcessAttachInfo &attach_info, Debugger &debugger,
                         Target *target, // Can be NULL, if NULL create a new
                                         // target, else use existing one
                         Status &error) override;

  bool GetSupportedArchitectureAtIndex(uint32_t idx, ArchSpec &arch) override;

  size_t GetSoftwareBreakpointTrapOpcode(Target &target,
                                         BreakpointSite *bp_site) override;

  bool GetRemoteOSVersion() override;

  bool GetRemoteOSBuildString(std::string &s) override;

  bool GetRemoteOSKernelDescription(std::string &s) override;

  // Remote Platform subclasses need to override this function
  ArchSpec GetRemoteSystemArchitecture() override;

  FileSpec GetRemoteWorkingDirectory() override;

  bool SetRemoteWorkingDirectory(const FileSpec &working_dir) override;

  // Remote subclasses should override this and return a valid instance
  // name if connected.
  const char *GetHostname() override;

  UserIDResolver &GetUserIDResolver() override { return *this; }

  bool IsConnected() const override;

  Status ConnectRemote(Args &args) override;

  Status DisconnectRemote() override;

  Status MakeDirectory(const FileSpec &file_spec,
                       uint32_t file_permissions) override;

  Status GetFilePermissions(const FileSpec &file_spec,
                            uint32_t &file_permissions) override;

  Status SetFilePermissions(const FileSpec &file_spec,
                            uint32_t file_permissions) override;

  lldb::user_id_t OpenFile(const FileSpec &file_spec, File::OpenOptions flags,
                           uint32_t mode, Status &error) override;

  bool CloseFile(lldb::user_id_t fd, Status &error) override;

  uint64_t ReadFile(lldb::user_id_t fd, uint64_t offset, void *data_ptr,
                    uint64_t len, Status &error) override;

  uint64_t WriteFile(lldb::user_id_t fd, uint64_t offset, const void *data,
                     uint64_t len, Status &error) override;

  lldb::user_id_t GetFileSize(const FileSpec &file_spec) override;

  Status PutFile(const FileSpec &source, const FileSpec &destination,
                 uint32_t uid = UINT32_MAX, uint32_t gid = UINT32_MAX) override;

  Status CreateSymlink(const FileSpec &src, const FileSpec &dst) override;

  bool GetFileExists(const FileSpec &file_spec) override;

  Status Unlink(const FileSpec &path) override;

  Status RunShellCommand(
      llvm::StringRef shell, llvm::StringRef command,
      const FileSpec &working_dir, // Pass empty FileSpec to use the current
                                   // working directory
      int *status_ptr, // Pass NULL if you don't want the process exit status
      int *signo_ptr,  // Pass NULL if you don't want the signal that caused the
                       // process to exit
      std::string
          *command_output, // Pass NULL if you don't want the command output
      const lldb_private::Timeout<std::micro> &timeout) override;

  void CalculateTrapHandlerSymbolNames() override;

  const lldb::UnixSignalsSP &GetRemoteUnixSignals() override;

  lldb::ProcessSP ConnectProcess(llvm::StringRef connect_url,
                                 llvm::StringRef plugin_name,
                                 lldb_private::Debugger &debugger,
                                 lldb_private::Target *target,
                                 lldb_private::Status &error) override;

  size_t ConnectToWaitingProcesses(lldb_private::Debugger &debugger,
                                   lldb_private::Status &error) override;

  virtual size_t
  GetPendingGdbServerList(std::vector<std::string> &connection_urls);

protected:
  process_gdb_remote::GDBRemoteCommunicationClient m_gdb_client;
  process_gdb_remote::GDBRemoteCommunicationReplayServer m_gdb_replay_server;
  std::string m_platform_description; // After we connect we can get a more
                                      // complete description of what we are
                                      // connected to
  std::string m_platform_scheme;
  std::string m_platform_hostname;

  lldb::UnixSignalsSP m_remote_signals_sp;

  // Launch the debug server on the remote host - caller connects to launched
  // debug server using connect_url.
  // Subclasses should override this method if they want to do extra actions
  // before or
  // after launching the debug server.
  virtual bool LaunchGDBServer(lldb::pid_t &pid, std::string &connect_url);

  virtual bool KillSpawnedProcess(lldb::pid_t pid);

  virtual std::string MakeUrl(const char *scheme, const char *hostname,
                              uint16_t port, const char *path);

private:
  std::string MakeGdbServerUrl(const std::string &platform_scheme,
                               const std::string &platform_hostname,
                               uint16_t port, const char *socket_name);

  llvm::Optional<std::string> DoGetUserName(UserIDResolver::id_t uid) override;
  llvm::Optional<std::string> DoGetGroupName(UserIDResolver::id_t uid) override;

  PlatformRemoteGDBServer(const PlatformRemoteGDBServer &) = delete;
  const PlatformRemoteGDBServer &
  operator=(const PlatformRemoteGDBServer &) = delete;
};

} // namespace platform_gdb_server
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PLATFORM_GDB_SERVER_PLATFORMREMOTEGDBSERVER_H
