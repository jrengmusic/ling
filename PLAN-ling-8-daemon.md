# PLAN: LING Part 8 — The Shared Daemon: Create or Join, One Instance, Its Own Lifetime

**RFC:** none — objective from ARCHITECT prompt. Canon: `PLAN-ling-0-tracker.md`, rulings 16, 19, 20, 21, 29, 44, 45, 46, 47, 48, 50, 52, 53, 58, 65; tracker Risks item 16; RFC ruling 4. Rulings 70 to 77.
**Date:** 2026-11-10. Updated 2026-11-13: lock-free at the hot path, no wait, no pending request (section The update of 2026-11-13).
**BLESSED Compliance:** verified
**Language Constraints:** C++17 / JUCE 8.0.14 / JAM (LANGUAGE.md, C++ / JUCE)

## Overview

Parts 6 and 7 read the manifest and parse every file in each run, and stop after one answer. This part keeps the documents in one process for each manifest and toolchain argument: the daemon (ruling 29). Clients reach it over 127.0.0.1 (ruling 46).
The daemon has the create-or-join form of `jam::SharedInstance` (ruling 47): "First holder's args construct; later holders join and their own args are discarded" (`jam/jam_core/utils/jam_SharedInstance.h:1-3`). A client joins when a daemon answers. Otherwise it starts one. Only one daemon runs for a manifest and a toolchain argument.
The daemon knows its own lifetime (ruling 48). Each request restarts its own timer. When the timer fires, the daemon ends. No request is ever pending: a request is answered inside its add event (section The update of 2026-11-13). The length of the timer has a default in LING. A row in the manifest of the project overrides it (ruling 71).
Inside the daemon, every change is an event on the state tree of Part 6 (ruling 45). A connection adds a `REQUEST` row. `Processor` writes `Id::result`. The connection sends it back.

## What this part does not do

- No LSP protocol. A client speaks the message form of this part. No part holds a stdio LSP wrapper (tracker Risks item 16).
- No completion. Part 9.
- No swap. Part 11 keeps the trees on disk (ruling 54).

## Language / Framework Constraints

- Ruling 47 and `jam_SharedInstance.h:1-16`: one daemon for each manifest and toolchain argument; create or join; the arguments of a later client are discarded.
- Ruling 48: *"ling no way to see an agent, but it know its own lifetime. multiple instance is not allowed. each handover whatever request is checking should it kept alive or kill itself"*. The length comes from a `project-info.md` row. It replaces "last drop destroys" of ruling 47. Ruling 71: *"daemon timeout should have default value, table definition override."*
- MANIFESTO **E**, Encapsulation rule 3: "Objects manage their own state". No client tells the daemon to end, and no client tracks it.
- Ruling 46: `juce::InterprocessConnectionServer` and `juce::InterprocessConnection` over 127.0.0.1, a port file, and a lock (ruling 29).
- Ruling 16: the same behavior on macOS and Windows. On Windows, ARCHITECT's shell is MSYS2 zsh. LING builds with MSVC or clang-cl, so `ling.exe` is a native Windows program, and the Windows API applies.
- Ruling 44 and `end/ARCHITECTURE.md:886-907`: "one sole AUTHOR writes the change; every listener reacts automatically on its own lane". Nothing is ported from `endless` byte for byte.
- MANIFESTO **B**: "Threads are bound to their context". The listener thread of the server only constructs a connection object and posts its ownership to the message thread. Every other touch of the tree, the documents, and the connections is on the message thread.
- MANIFESTO **B**, The Guard Rule: every order of destruction in this part names its threat.
- Lock-free at the hot path, no wait, fully event driven. ARCHITECT, 2026-11-13: *"lock free at hot path is AUDIO ARCHITECTURE DISCIPLINE. i have alrady explicitly said, all jam project adhere to it"*; *"i thought the whole contract is lock-free at hot path, no wait, fully event driven?"*; *"why wait? not event?"*. Ruling 44: *"fully event driven"*.
- MANIFESTO, Permitted copies, item 2: an ownership handoff between threads "replaces a lock or a wait". The established form is `Processor::addJob` (`ling/Source/Processor.cpp:507-512`): a `std::make_shared` holder through `juce::MessageManager::callAsync`.
- MANIFESTO **E**, Complete at Creation: "Each consumer downstream receives complete data." "A complete state replaces a complete state."
- CODING.md: no includes in submodule files, except platform headers in a native file; no forward declaration; no fake-carrier struct; no `==` on strings; no out-parameter.
- CODING.md: use the framework API fully. JAM first, then JUCE.
- CAROL.md Code Hygiene: no comment and no doxygen in this sprint.
- `jam/.claude/CLAUDE.md:104`: "Agents never build. ARCHITECT builds."
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Dependency & API Inventory

Line numbers are reads of 2026-11-10. JUCE paths are below `~/.cache/cast/juce/8acf824d884a4e9a0d21b7099dbc5131f4df8b01ae04ada5fe310faad2b215fd/modules/`, the checkout that END builds with.

### What exists, and what does not

- END and JAM have no transport between processes. The doxygen indexes `end/docs/xml` and `jam/docs/xml` name no `InterprocessConnection`, `NamedPipe`, `StreamingSocket`, or `ChildProcess` user. `end/ARCHITECTURE.md` and `end/SPEC.md` name no daemon and no socket.
- JUCE has the transport: `juce_events/interprocess/juce_InterprocessConnection.h`, `juce_InterprocessConnectionServer.h`.
- No JUCE or JAM API starts a child that is detached from the terminal and the process group of its parent:
  - JUCE `ChildProcess` on POSIX forks with no `setsid` and sends stdout and stderr to `/dev/null` (`juce_core/native/juce_SharedCode_posix.h:1122-1154`). On Windows it uses `CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT` (`juce_core/native/juce_Threads_windows.cpp:451-453`).
  - `jam::ChildProcess` pipes stdout and stderr of the child to the parent (`jam/jam_subprocess/subprocess/native/jam_ChildProcess_posix.cpp:45-58`; `jam_ChildProcess_windows.cpp:14-29`).
  - The only detached start in ARCHITECT's code is in `endless`: `posix_spawn` with `POSIX_SPAWN_SETSID` and `/dev/null` streams (`endless/Source/nexus/Daemon.mm:40-64`), and `CreateProcessW` with `DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | CREATE_UNICODE_ENVIRONMENT | CREATE_BREAKAWAY_FROM_JOB` (`endless/Source/nexus/DaemonWindows.cpp:42-70`).

### Existing pieces

| Need | Existing piece | Where |
|---|---|---|
| The ownership form | `jam::SharedInstance`: `create` under `createLock()`; `weak()` holds a reference that does not own; `weak().lock()` joins; `std::make_shared` when no object is live; last drop destroys | `jam/jam_core/utils/jam_SharedInstance.h:1-16`, `:44-59`, `:91-101` |
| `Instance` is a lookup, not an owner | "it grants scoped global lookup, it does not own" | `jam/jam_core/utils/jam_Instance.h:20-25` |
| A server for socket clients | `juce::InterprocessConnectionServer`: `beginWaitingForSocket (portNumber, bindAddress)`; `stop`; `getBoundPort`, "useful ... when calling beginWaitingForSocket with a port number of zero"; `createConnectionObject`, called "by the listener thread" | `juce_events/interprocess/juce_InterprocessConnectionServer.h:63-107` |
| One connection | `juce::InterprocessConnection (callbacksOnMessageThread, magicMessageHeaderNumber = 0xf2b49e2c)`; `connectToSocket (hostName, portNumber, timeOutMillisecs)`; `disconnect`; `sendMessage (MemoryBlock)`; `connectionMade`, `connectionLost`, `messageReceived` on the message thread when `callbacksOnMessageThread` is true | `juce_InterprocessConnection.h:69-201` |
| The rule for a derived connection | "Your derived Connection class *must* call `disconnect` in its destructor" | `juce_InterprocessConnection.h:58-59`; `juce_InterprocessConnection.cpp:94-106` |
| Delete a connection in its own `connectionLost` | the callback runs inside `SafeActionImpl::ifSafe`, which holds a `juce::CriticalSection` and touches nothing of the connection after `fn (ref)` returns; `disconnect` in the destructor takes the same lock on the same thread | `juce_InterprocessConnection.cpp:47-78`, `:160-178`, `:261-282` |
| The local address | `juce::IPAddress::local()`; JUCE itself returns it for a local connection | `juce_InterprocessConnection.cpp:208` |
| One process for one name | `juce::InterProcessLock (name)`, `enter (timeOutMillisecs)`; on Windows an abandoned mutex counts as acquired | `juce_core/threads/juce_InterProcessLock.h`; `juce_Threads_windows.cpp:334-369` |
| A server that owns its connections | `endless`: `connections.add (std::make_unique<Channel> (...)).get()` in `createConnectionObject`; `stop()` first in the destructor. Its `connections` has no lock, but the listener thread adds and the message thread removes. LING does not take this form: its listener thread adds nothing (section The update of 2026-11-13). | `endless/Source/nexus/Daemon.cpp:34-38`, `:104-117`; `Daemon.h:389` |
| The listener thread calls `createConnectionObject`, then `initialiseWithSocket` on the returned object; `connectionMade` is posted after that | `if (auto* newConnection = createConnectionObject()) newConnection->initialiseWithSocket (std::move (clientSocket));`; `connectionMadeInt(); thread->startThread();` | `juce_events/interprocess/juce_InterprocessConnectionServer.cpp:87-88`; `juce_InterprocessConnection.cpp:242-243` |
| A handoff of ownership to the message thread | `std::make_shared<std::unique_ptr<...>>` captured into `juce::MessageManager::callAsync` | `ling/Source/Processor.cpp:507-512` |
| A pool of one thread runs its jobs in order | `juce::ThreadPool (const Options&)`; `ThreadPoolOptions::withNumberOfThreads (int)` | `juce_core/threads/juce_ThreadPool.h:177`, `:223` |
| A text parameter written from any thread | `ParameterText::setValue`: "Lock-free producer-side store ... Any thread."; `ParameterAttachment` runs its callback at once on the message thread, and through `triggerAsyncUpdate` from any other thread | `jam/jam_data_structures/model/jam_ParameterText.h:60`; `jam_Model.cpp:220-234` |
| A synchronous connect | `s->connect (hostName, portNumber, timeOutMillisecs)` inside `connectToSocket`. JUCE gives no asynchronous connect. | `juce_InterprocessConnection.cpp:116` |
| An owning container | `jam::Owner<ObjectClass>`: `add`, `remove`, `indexOf`, `size`. It has no lock. | `jam/jam_core/utilities/jam_Owner.h:94`, `:219`, `:279`, `:449` |
| A reference to the global model as a member | `ENDModel& appModel { *ENDModel::getInstance() };` | `end/Source/config/ConfigModel.h:115` |
| An object that holds its row as `state` | `theme.state`, `background.state` | `end/Source/config/ConfigModel.cpp:25-29` |
| A reaction to any event of the watcher | "Any event type calls `loadFromPath()`" | `end/Source/config/ConfigModel.h:147-155`; `ConfigModel.cpp:123-127` |
| The six events of the watcher | `undefined`, `fileCreated`, `fileDeleted`, `fileUpdated`, `fileRenamedOldName`, `fileRenamedNewName`; recursive on macOS and Windows | `jam/jam_core/file/jam_Listener.h:38-41`, `:105-118` |
| The folder of a tool on the machine | cast: `$ENV{LOCALAPPDATA}/cast/juce` on Windows, `$ENV{HOME}/.cache/cast/juce` otherwise. Ruling 58 gives LING the same shape: `~/.cache/ling/`. | `jam/cast/juce.cast:8-12` |
| `%LOCALAPPDATA%` | `juce::File::windowsLocalAppData`, "On a Windows machine returns the %LOCALAPPDATA% folder"; the value exists only inside `#if JUCE_WINDOWS` | JUCE `juce_core/files/juce_File.h:993-995` |
| Make a folder before a write | `file.getParentDirectory().createDirectory();` | `jam/jam_markdown/document/jam_ConfigDocument.cpp:747` |
| A file name from a stem and an extension | `jam::Format::toFileName (juce::StringRef name, juce::StringRef extension)` | `jam/jam_core/text/jam_Format.cpp:141-146`; `end/Source/config/ConfigDirectory.cpp:16` |
| A number as text, appended to a `std::string` | `static void appendDecimal (std::string& text, int value);`: "converts the value with `std::to_chars` into a stack buffer and appends the digits". `jam::Format` has no function for the hexadecimal digits of a number: `toHex (const juce::String&)` converts one character to a `0x` text. | `jam/jam_core/text/jam_Format.h:404-412`, `:862` |
| A 64-bit hash of bytes | `static std::uint64_t jam::Wyhash::hashBytes (const void* key, std::size_t len) noexcept` | `jam/jam_core/utils/jam_HashMap.h:221` |
| The UTF-8 bytes of a `juce::String` | `jam::Strings::getView (const juce::String&)`, public and static | `jam/jam_core/text/jam_Strings.h:633` |
| The argument quote rule of Windows | `getQuotedArgument`, file-static | `jam_ChildProcess_windows.cpp:115-145` |
| The argv build of POSIX | the loop in `Native`; it takes each pointer with `const_cast<char*>` (`:24`) | `jam_ChildProcess_posix.cpp:20-26` |
| The connect timeout | `killProbeTimeoutMs { 200 }`, `nexusProbeTimeoutMs { 200 }` | `endless/Source/Main.cpp:98`, `:329` |
| A timer that a new start resets, and that its owner stops | `juce::Timer::startTimer`: "If the timer is already started, this will reset it"; `stopTimer`; `timerCallback` | `juce_events/timers/juce_Timer.h:85-119` |
| The timer of `jam::Model` is taken | the flush timer: "first tick at 10 Hz, then 120 Hz ... and 60 Hz otherwise" | `jam/jam_data_structures/model/jam_Model.h:8`, `:30-41` |
| A `name`, `value`, `description` table in `project-info.md` | `## include` | `end/project-info.md:398-412` |
| JSON | `juce::JSON::parse`, `juce::JSON::toString` | `whatdbg/Source/dap/Reader.cpp:98` |
| The module row | `\| @user-module \| jam_subprocess \| ... \|` | `scout/project-info.md:142` |
| The state tree, the documents, the request row | `LINGModel`, `Processor`, `REQUEST` with `Id::command`, `Id::arguments`, `Id::result` | PLAN-ling-6-manifest.md, The design; PLAN-ling-7-queries.md, The nine operations |

### Names

| Name | Source |
|---|---|
| `daemon`, the command word and the identifier | ruling 29, *"Shared daemon"* |
| `Server`, `Connection` | JUCE, verbatim: `InterprocessConnectionServer`, `InterprocessConnection` |
| `Client` | the word of ruling 29: "one process, many clients" |
| `connections` | `endless/Source/nexus/Daemon.h:389` |
| `addManifestJob`, `manifestPool`, `manifestThreadCount` | `addJob` (`ling/Source/Processor.h:808`); `pool` (`Processor.h:861`) |
| `setFiles` | `setDocuments`, `setResult` (`ling/Source/Processor.h:846`, `:625`) |
| `getTimeout` | the verb `get`; `Id::timeout` |
| `Server::getPortFile() const`, the private overload that reads the two values from the root | `getPortFile` |
| `Server::state`, the `CONNECTIONS` row | `theme.state` (`ConfigModel.cpp:25`) |
| `Client::start` | `Server::start`; `ChildProcess::start` |
| `Client::connectToDaemon` | `connectToSocket`, `connectToPipe` (JUCE) |
| `Client::request`, the message text; `Client::manifestFile`, `Client::toolchainArgument` | the `REQUEST` row; `Id::manifest`, `Id::toolchainArgument` |
| `runDaemon`, `runOperation` | `runCommandLine`, `runDocumentNotFound` (`Main.cpp:159`, `:241`) |
| `Id::connections`, `Id::connection` | the plural row and the singular row, as `Id::files` and `Id::file` (PLAN-ling-6-manifest.md) |
| `state` | `theme.state` (`ConfigModel.cpp:25`) |
| `model` | `appModel` (`ConfigModel.h:115`), without the `app` of END |
| `jam::ChildProcess::startDetached` | `jam::ChildProcess::start` (`jam_ChildProcess.h:44`); `DETACHED_PROCESS` (`DaemonWindows.cpp:57`); "The View is detachable" (`end/ARCHITECTURE.md:834`) |
| `getArgumentBytes`, `getArgumentPointers` | the verb `get`; the bytes and the `char*` array that `execvp` and `posix_spawn` read |
| `LINGModel::getDaemonName`, `Server::getPortFile` | the verb `get`; `getPortFile` in the form of `ConfigDirectory::Config::getPath` |
| `LINGModel::getCacheDirectory` | the verb `get`; `CAST_JUCE_CACHE_DIR` (`jam/cast/juce.cast:9-11`); ruling 58 |
| `jam::Format::appendHex` | `appendDecimal` (`jam_Format.h:412`); `toHex` (`jam_Format.h:862`) |
| File rows `lingPort`, `lingCache`, `lingLocalCache` | `lingHelp` (PLAN-ling-1-project.md Step 6) |
| `connectTimeoutMs`, `anyPort` | `coalesceMs` (`ConfigModel.h:164`); `nexusProbeTimeoutMs` |
| `defaultTimeoutMs` | `connectTimeoutMs`; a JAM default that a config row overrides: `defaultSequenceTimeout` (`end/ARCHITECTURE.md:895-897`) |
| `## daemon` table, row `timeout`; `Id::timeout` | the command word `daemon`; `timeoutMilliseconds` (`jam/jam_subprocess/subprocess/jam_ChildProcess.h:61-66`); `timeOutMillisecs` (`juce_InterprocessConnection.h:99`) |
| Text rows `failDaemonStart`, `failDaemonLost` | `failUnitNotFound` (PLAN-ling-6-manifest.md Step 4) |
| Rule key `daemon`; text row `failDaemonTimeout` | scout's rule key `sample`, the table word (`scout/Source/Validator.cpp:27`); cast's `failFormatWidth`, the table word and then the checked word (`cast/cast/text.md:55`) |

## The design

### The ownership form

| `jam::SharedInstance` | The LING daemon |
|---|---|
| one live object for each type | one daemon for each pair of a manifest and a toolchain argument. Its name is `LINGModel::getDaemonName (manifest, toolchainArgument)`. The pair is the input of the daemon: another toolchain argument reads other compiler manifests (PLAN-ling-6-manifest.md, In LING: the compiler manifest), so it is another daemon. MANIFESTO **D**: "The same input must always yield the same output". |
| `createLock()` (`:91-95`) | `juce::InterProcessLock` with the daemon name. The daemon holds it for its whole life. |
| `weak()`, a reference that does not own (`:97-101`) | the port file. It holds the port and owns nothing. |
| `weak() = object` after the object exists (`:55-56`) | the daemon writes the port file after `beginWaitingForSocket` returns |
| `weak().lock()` gives a live object: join (`:51`) | `connectToSocket` succeeds |
| no live object: `std::make_shared` (`:53-56`) | the client starts the daemon with `jam::ChildProcess::startDetached` |
| "later holders join and their own args are discarded" (`:2-3`) | a client that joins a live daemon does not send its `project-info.md`; the daemon keeps the one it started with |
| "last drop destroys" | replaced by ruling 48: the daemon ends by its own timer |

### In JAM: the detached start

`static bool startDetached (const jam::Strings& arguments);` (ruling 50) is a new static member of `jam::ChildProcess` (`jam_ChildProcess.h:15-93`). It is static, because the object does not own the child: a detached child has no owner in the parent process. Index 0 of `arguments` is a full executable path. It returns `true` when the child starts.

| Platform | The start |
|---|---|
| POSIX (`jam_ChildProcess_posix.cpp`) | `posix_spawn` with the attribute flag `POSIX_SPAWN_SETSID`. File actions open `/dev/null` as stdin (read) and as stdout and stderr (write). The environment is `environ`. The argv comes from `getArgumentBytes` and `getArgumentPointers`, two file-static functions that replace the loop of `Native` (`:20-26`). `Native` and `startDetached` both call them. |
| Windows (`jam_ChildProcess_windows.cpp`) | `CreateProcessW` with no inherited handles and the flags `DETACHED_PROCESS \| CREATE_NEW_PROCESS_GROUP \| CREATE_UNICODE_ENVIRONMENT \| CREATE_BREAKAWAY_FROM_JOB`. The command line comes from `getQuotedArgument` (`:115-145`), as `start` builds it (`:157-160`). Both handles of `PROCESS_INFORMATION` are closed at once. |

`setsid` gives the child its own session and process group, with no controlling terminal. So a Ctrl-C or a closed terminal of the client does not reach the daemon. On Windows the flags do the same, and `CREATE_BREAKAWAY_FROM_JOB` takes the child out of a job of the client.

### In LING: the state tree

Part 6 gives the tree. This part adds the `CONNECTIONS` row:

```
LING (Id::ling)                     Id::toolchainArgument, Id::manifest, Id::documents, Id::timeout
  FILES (Id::files)                 Id::compilerManifests, Id::dependencyRecords
    FILE (Id::file)                 Id::path, Id::family, Id::output, Id::dependencies
  CONNECTIONS (Id::connections)
    CONNECTION (Id::connection)
      REQUEST (Id::request)         Id::command, Id::arguments, Id::result
  OVERLAY (Id::overlay)             Id::message
```

| Row or property | Author | Reader |
|---|---|---|
| `Id::timeout` on the root | `LINGModel`: `defaultTimeoutMs` in the constructor; then `Processor::setFiles`, at the end of each read of the manifest: the value that `Processor::getTimeout` gives, the `timeout` row of the `## daemon` table when the manifest holds one, and `defaultTimeoutMs` when it holds none | `Server`, at each start of its timer |
| `FILES` row | `Processor::setFiles`, on the message thread, at the end of each manifest job (section The update of 2026-11-13, C1) | `Processor`; `LINGModel`, for the watcher |
| `Id::compilerManifests`, `Id::dependencyRecords` on the `FILES` row | `CompilerManifest`, when it makes the row | `LINGModel`, for the watcher |
| a `CONNECTION` row | its `Connection`, in `connectionMade`; removed by it in `connectionLost` | `Connection` |
| a `REQUEST` row | its `Connection`, in `messageReceived`, with `Id::command`, `Id::directory`, and `Id::arguments`; removed by it after it sends the result, inside the add event | `Processor` (Part 7), which runs it at once; `Server`: each addition restarts the timer |
| `Id::result` | `Processor` (Part 7) | the `Connection` of the row |

A `REQUEST` row is a child of the `CONNECTION` row of its client. So the removal of a `CONNECTION` row also removes its rows. `Processor` listens to the root, and a `juce::ValueTree` listener gets the child events of every row below it. Part 7 does not hold without change: its pending path scans only the direct children of the root (`Processor.cpp:557-563`). The update of 2026-11-13 deletes that path (C2).

### In LING: `Server` and `Connection`

`LINGModel` gets two static members. `Server`, `Client`, and `Processor` (Part 10) call them. So no object names another object for the name of a daemon or for a folder (MANIFESTO **E**).

- `static std::string getDaemonName (const juce::File& manifest, std::string_view toolchainArgument);` returns the text that `jam::Format::appendHex` (Step 1 item 8) makes from `jam::Wyhash::hashBytes` over one `std::string` work buffer. The buffer holds the bytes of `jam::Strings::getView` of `manifest.getFullPathName()`, then `Chars::newline`, then the bytes of the toolchain argument. `getFullPathName` gives a `juce::String`, a JUCE output (ruling 50). The function holds it in a local, so the view stays valid for the copy. `juce::InterProcessLock` takes the name as a `juce::String` argument (ruling 50: final output).
- `static juce::File getCacheDirectory();` returns the folder of LING on the machine (ruling 58): `juce::File::getSpecialLocation (juce::File::userHomeDirectory)` plus `files::lingCache`. Inside `#if JUCE_WINDOWS` it returns `juce::File::getSpecialLocation (juce::File::windowsLocalAppData)` plus `files::lingLocalCache`. This is the form of cast's root (`jam/cast/juce.cast:8-12`). It makes no folder: a `get` returns its object (NAMES.md Rule 1, Verb Contract). An object that writes into the folder, or that watches it, makes it first with `createDirectory()` (`jam_ConfigDocument.cpp:747`).

`class Server : public juce::InterprocessConnectionServer, private juce::Timer, private juce::ValueTree::Listener` owns the connections and the lifetime of the daemon:

- `jam::Owner<Connection> connections;`. Only the message thread touches it. No lock exists.
- `createConnectionObject()` runs on the listener thread (`juce_InterprocessConnectionServer.h:98-107`). It makes `auto connection { std::make_shared<std::unique_ptr<Connection>> (std::make_unique<Connection> (*this)) };`, posts `juce::MessageManager::callAsync ([this, connection] { connections.add (std::move (*connection)); })`, and returns the raw object to JUCE. This is the handoff of `Processor::addJob` (`Processor.cpp:507-512`). JUCE posts `connectionMade` only after this function returns (`juce_InterprocessConnectionServer.cpp:87-88`; `juce_InterprocessConnection.cpp:242`), so the ownership message is first in the queue. When the message loop ends before the lambda runs, the destruction of the lambda deletes the `Connection`, and its destructor calls `disconnect()`.
- `Server` has no `removeConnection`. Each `Connection` holds `jam::Owner<Connection>& connections`, the container of `Server`, and removes itself from it as the last statement of `connectionLost`, on the message thread. CODING.md, No Forward Declarations: `Server.h` includes `Connection.h` for its `jam::Owner<Connection>` member, so `Connection.h` cannot include `Server.h`, and a `Server&` member would need a forward declaration.
- Member `LINGModel& model { *LINGModel::getInstance() };` (`ConfigModel.h:115`). The constructor adds `Server` as a listener of the model, in the form of `Processor.cpp:6`; the destructor removes it.
- Member `juce::ValueTree state { model.state.getChildWithName (jam::Format::toType (Id::connections)) };`, the `CONNECTIONS` row (`theme.state`, `ConfigModel.cpp:25`).
- `valueTreePropertyChanged` of `Id::documents` on the root, the first one only: make the folder with `LINGModel::getCacheDirectory().createDirectory()`, write the port file with `replaceWithText (juce::String (getBoundPort()))`, call `startTimer` with the value of `Id::timeout`, then `model.removeListener (this)` and `state.addListener (this)`. The change of listener makes "the first" an event, with no flag. So no client finds the daemon before its first complete set of documents, a daemon that no client reaches also ends, and a later reload or file save neither rewrites the port file nor restarts the timer ("Each request restarts its own timer").
- `valueTreeChildAdded` of a `REQUEST` row, through `state`, calls `startTimer` with the value of `Id::timeout`. A start resets a running timer (`juce_Timer.h:93-97`).
- `timerCallback` calls `stop()` first, then deletes the port file, calls `stopTimer`, and stops the message loop. The threat of `stop()` first: after `quit()`, `MessageBase::post` deletes a posted message at once (`juce_MessageManager.cpp:85-88`), so a handoff of a connection that comes after the quit would free the `Connection` before JUCE calls `initialiseWithSocket` on it (`juce_InterprocessConnectionServer.cpp:87-88`). It tests nothing else: a `REQUEST` row never outlives its add event (section The update of 2026-11-13, C2).
- The destructor calls `stop()` first. The threat: the base class stops the listener thread only after the members are destroyed, so the listener thread could post a handoff after `connections` is destroyed.
- `void start();` listens: `beginWaitingForSocket (anyPort, juce::IPAddress::local().toString())`, with `static constexpr int anyPort { 0 };`.
- `static juce::File getPortFile (const juce::File& manifest, std::string_view toolchainArgument);` returns the file `jam::Format::toFileName (LINGModel::getDaemonName (manifest, toolchainArgument), files::lingPort)` in `LINGModel::getCacheDirectory()`. `Server` takes the two values from `Id::manifest` and `Id::toolchainArgument` on the root each time it needs the port file. It keeps no copy (MANIFESTO **S**, shadow state).

`class Connection : public juce::InterprocessConnection, private juce::ValueTree::Listener` is one client inside the daemon:

- It is made with `juce::InterprocessConnection (true)`. So every callback is on the message thread. The magic number is JUCE's default (CODING.md: configure only the difference).
- Members: `jam::Owner<Connection>& connections;`, `LINGModel& model { *LINGModel::getInstance() };` (`ConfigModel.h:115`), and `juce::ValueTree state;`. The constructor runs on the listener thread and touches no tree.
- The destructor removes `Connection` as a listener of `state`, then calls `disconnect()` (`juce_InterprocessConnection.h:58-59`). The threat of the removal: when `Server` is destroyed, the `CONNECTION` rows stay in the tree, and a row would keep a listener that no longer exists.

| Event | Reaction |
|---|---|
| `connectionMade` | `state` becomes a new `CONNECTION` row. It is appended to `CONNECTIONS`. `Connection` listens to `state`. |
| `messageReceived (message)` | `juce::JSON::parse (message.toString())` gives an array of strings. A `REQUEST` row is appended to `state`, with `Id::command` the first string, as text (`Processor` makes the identifier, PLAN-ling-7-queries.md, The nine operations), `Id::directory` the second string, the working folder of the client (`Processor.h:530-540`), and `Id::arguments` the other strings. `Processor` answers it inside this add event. |
| `valueTreePropertyChanged` of `Id::result` on a `REQUEST` row | the text of `Id::result` goes back with `sendMessage`. The row is removed from `state`, inside the same add event. |
| `connectionLost` | `state` is removed from `CONNECTIONS`. Then `connections.remove (connections.indexOf (this))`, the last statement, deletes the connection. |

### In LING: the daemon application

`ling daemon <project-info.md> [--<word>]`:

1. Make the message loop, as Part 6 Step 6 makes it.
2. Make `juce::InterProcessLock lock { LINGModel::getDaemonName (manifest, toolchainArgument) };`. When `lock.enter (0)` is `false`, another daemon serves this manifest and this toolchain argument: exit with code 0 at once. A client that started this process joins that daemon through the port file.
3. Make `Server`. `LINGModel` and `Processor` are members of the application already. Write `Id::toolchainArgument`, then `Id::manifest` (Part 6). The manifest job of `Processor` starts (section The update of 2026-11-13, C1).
4. `server.start()` listens. The port file comes at the first `Id::documents` (section In LING: `Server` and `Connection`).
5. When the message loop ends, exit with code 0. The destructor of `Server` stops the listener thread and deletes the connections. A client that still holds a connection gets `connectionLost`.
6. Each message goes to stderr through the one `jam::Model::ParameterAttachment` that the application holds (PLAN-ling-6-manifest.md, In LING: the application, item 2).
7. The application adds itself as a listener of the model only in the manifest run of Part 6. So the daemon does not print and quit at its first `Id::documents` (`Main.cpp:406-417`). The daemon run holds `std::unique_ptr<juce::InterProcessLock> lock` and `std::unique_ptr<Server> server`, made in its branch of `initialise`.

**The `## daemon` table.** LING declares this table. A project holds it in its `project-info.md` only to override the default (rulings 71 and 72). `LINGModel` holds `static constexpr int defaultTimeoutMs { 60000 };` (ruling 53) and writes it to `Id::timeout` in its constructor. The manifest job of `Processor` (section The update of 2026-11-13, C1) reads the table from the document that it already holds, with `static int Processor::getTimeout (const jam::ConfigDocument& manifest)`, in the form of `scout/Source/Processor.cpp:36-55`, before it calls `CompilerManifest::getFiles`. The table id is the text of `Id::daemon`. The table has the columns `name`, `value`, `description`. One row: `timeout`, its value in milliseconds. At each read, `Processor::setFiles` writes `Id::timeout`: the value of the row, or `defaultTimeoutMs` when the manifest holds no table or no row. `HELP.md` states the default and the table. Part 11 adds its own read to this reaction. `Validator` (PLAN-ling-6-manifest.md, In LING: the manifest check) gets one more rule in this part (ruling 79). Its key is `Id::daemon.toString()`, the table word, as scout's rule `sample` (`scout/Source/Validator.cpp:27`). For each table of `config.getTables (Id::daemon)` that holds a `timeout` row (`config.getTableRow (*table, Id::timeout) != nullptr`), it reads `config.getTableValue (*row, Id::value)` and fails a value that is not an integer greater than zero. The test is cast's test of a `width` cell: `juce::String (value.getIntValue()).compare (value) != 0 or value.getIntValue() <= 0` (`cast/Source/Validator.cpp:948-950`). The failure text is `getLocation (*table, *row, Id::value.toString())`, then `Id::diagnosticSeparator`, then the text row `failDaemonTimeout`. A manifest with no `## daemon` table passes, and a table with no `timeout` row passes: neither is a failure, and `Id::timeout` then holds `defaultTimeoutMs`. ARCHITECT: *"daemon timeout undeclared does not mean failure, it should have internal default. table declaration at project-info.md is optional only to override"*. The rule judges only a value that a project declares. When the check of the manifest fails, the job reads no table and gives `defaultTimeoutMs` to `setFiles`. The job thus reads the `timeout` value with no test of its own (ruling 69).

The read of the manifest starts in step 3, on the one thread of `manifestPool`. The parse of the files runs in the pool of `Processor`. The port file comes at the first `Id::documents`, after the parse ends. No request is ever pending.

The process holds the lock (step 2) before the port file exists (the first `Id::documents`). So two daemons of one manifest and one toolchain argument never write two port files.

### In LING: the watched manifest

The daemon stays, so the manifest, the compiler manifests, and the dependency records can change under it (ruling 77). `CompilerManifest` gives each file that a read depends on to the new `FILES` row. The C family entry appends the full path of each compiler manifest that it reads to `Id::compilerManifests` of the row. After a run of the `ninja` row that exits with 0, it appends the full path of `<build folder>.getChildFile (text::CLanguage::dependencyRecord)` to `Id::dependencyRecords` of the row (PLAN-ling-6-manifest.md, The C family entry, The files, items 1 and 3). Each is a `juce::var` array, as `Id::dependencies` is. The row `dependencyRecord` of `cast/c_language.md` is `.ninja_deps`: ninja stores the record "as `.ninja_deps` in the `builddir`" (ninja manual), and for END it is in the build folder (`end/Builds/Debug/.ninja_deps`). LING watches that file and never reads it (ruling 76).

`LINGModel` adds the parent directory of the manifest to its watcher when `Id::manifest` is written, and the parent directory of each file of `Id::compilerManifests` and of `Id::dependencyRecords` when the `FILES` row is added. Any event for the manifest file, or for a file that one of the two properties of the `FILES` row holds, calls `sendPropertyChangeMessage (Id::manifest)` on the root. The manifest job of `Processor` then reads the manifest again, and `setFiles` replaces the `FILES` row (section The update of 2026-11-13, C1).

### In LING: the client

`class Client : public juce::InterprocessConnection, private jam::File::Watcher::Listener` is the other end. It is made with `juce::InterprocessConnection (true)`. Its destructor calls `disconnect()`. It has no timer. ARCHITECT, 2026-11-13: *"why wait? not event?"*. Each action of the client is a reaction to one event.

`ling <operation> <project-info.md> [--<word>] <arguments>` (Part 7):

1. Make the message loop. Make `Client`. It makes the folder with `LINGModel::getCacheDirectory().createDirectory()`, adds it to its watcher, and calls `coalesceEvents (coalesceMs)`. The application holds it as `std::unique_ptr<Client> client`, made in the operation branch of `initialise`.
2. Connect: read the port file, `Server::getPortFile (manifest, toolchainArgument)`, with `loadFileAsString().getIntValue()`. Call `connectToSocket (juce::IPAddress::local().toString(), port, connectTimeoutMs)`, with `static constexpr int connectTimeoutMs { 200 };` (`endless/Source/Main.cpp:98`).
3. When `connectToSocket` gives `false`, start the daemon: `jam::ChildProcess::startDetached` with these arguments: the full path of this executable; the word `daemon`; the manifest; and, when the toolchain argument is not empty, one argument of `Chars::doubleDash` and the toolchain argument. The executable path comes from `juce::File::getSpecialLocation (juce::File::currentExecutableFile)`. The word `daemon` is `Id::daemon`. A `false` from `startDetached` is a message with `failDaemonStart`, and exit code 1.
4. `fileChanged` for the port file, for any event, runs item 2 again (`ConfigModel.h:147-155`). So a new port file, a changed one, and a deleted one each give one more try. A successful connect removes `Client` from the listeners of its watcher at once. The threat: `connectToSocket` calls `disconnect()` first (`juce_InterprocessConnection.cpp:112`), so a port file event between a successful connect and `connectionMade` would end the connection.
5. `connectionMade`: send `juce::JSON::toString` of the array of the operation, the working folder of the client (`juce::File::getCurrentWorkingDirectory().getFullPathName()`, as `Main.cpp:386` gives it today), and the arguments. The manifest and the toolchain argument are not in the message: the daemon has them.
6. `messageReceived`: print the text to stdout, call `disconnect (-1, juce::InterprocessConnection::Notify::no)`, and stop the message loop. Exit code 0.
7. `connectionLost` before a result: a message with `failDaemonLost`, and exit code 1.

`failDaemonStart` comes only from a `false` of `startDetached`. The client waits for the next event with no bound. A daemon that dies before it writes its port file sends no event, so the client waits until Ctrl-C. `connectToSocket` is synchronous in JUCE (`juce_InterprocessConnection.cpp:116`), and JUCE gives no asynchronous connect. The client calls it once on each port file event, with `connectTimeoutMs`.

The in-process request path of Part 7 Step 5 item 5 is deleted first (CAROL.md Refactor-Rewrite Discipline). The `Connection` of the daemon is now the author of `REQUEST` rows. `ling <project-info.md> [--<word>]` of Part 6 stays in process: it is the cold load and its measurement.

### Why each race ends in one daemon

| Case | What happens |
|---|---|
| Two clients find no daemon | Both start one. One daemon takes the lock and writes the port file at its first `Id::documents`. The other exits in step 2. The watchers of both clients see the port file, and both connect to the one daemon. |
| A daemon crashed, and its port file stays | `connectToSocket` fails. The client starts a daemon. The OS released the lock of the dead process, so the new daemon takes it. It replaces the port file, and the watcher of the client sees it. |
| A client connects while the daemon ends | The daemon deletes the port file before it stops. The connect fails, or the connection is lost. A connect failure starts a new daemon. A lost connection is `failDaemonLost`. |
| A request comes while the timer runs | The addition of the `REQUEST` row restarts the timer. |
| A request comes during a reload | `Processor` answers it at once from the stored documents. Each document is complete: the old document of a file stays until its job replaces it (C2). |
| The timer fires while a reload runs | The daemon ends. No request is pending (C2). The manifest job and the parse jobs end with the pools. |

### The update of 2026-11-13

ARCHITECT: *"lock free at hot path is AUDIO ARCHITECTURE DISCIPLINE. i have alrady explicitly said, all jam project adhere to it"*; *"smell. what is pending request? why there is pending request? why pending? why Processor wait?"*; *"update the plan"*. The locked text had four points against that rule. Each is corrected in its section above, or in C1 and C2 below.

1. `connectionsLock`: a lock on each connection, and each client call is one connection. Corrected in section In LING: `Server` and `Connection`: the handoff through `callAsync`.
2. The pending request: `Processor::setDocuments` scans the root's direct children (`Processor.cpp:557-563`), and a request runs only when `pendingJobs == 0` (`:478`). A `REQUEST` row under `CONNECTION` is never found. Corrected by C2.
3. The client folder: `Processor::getPathIndex` reads `Id::directory` (`Processor.cpp:73`). The `REQUEST` row of the daemon had no such property. Corrected in sections `Connection` and In LING: the client: the message carries the folder.
4. `finished.wait()` for `ninja -t deps` (`CompilerManifest.cpp:149`) blocks the message thread inside the reaction to `Id::manifest` (`LINGModel.cpp:36`). The watched manifest runs that reaction at each build. Corrected by C1.

**C1. The read of the manifest leaves the message thread.**

- `Processor` gets `juce::ThreadPool manifestPool { juce::ThreadPool::Options{}.withNumberOfThreads (manifestThreadCount) };`, with `static constexpr int manifestThreadCount { 1 };` (`juce_ThreadPool.h:177`, `:223`). One thread runs the jobs in order, and `callAsync` keeps that order. So a later read never lands before an earlier one.
- `Processor::valueTreePropertyChanged`: `Id::manifest` on the root calls `void addManifestJob();`. The job takes the manifest path and the toolchain argument as values, as `addJob` takes its path (`Processor.cpp:501`).
- The job body is the body that `LINGModel.cpp` holds in its reaction to `Id::manifest` before this update, in this order:
  1. `jam::ConfigDocument::parse`.
  2. `Validator {}.isValid`. A failure calls `model.setMessage` and gives an empty `FILES` row and `LINGModel::defaultTimeoutMs`.
  3. `getTimeout (manifest)`.
  4. `CompilerManifest::getFiles`.

  The wait for `ninja` (`CompilerManifest.cpp:149`) then runs on `manifestPool`, never on the message thread. `CompilerManifest` does not change. `model.setMessage` is valid from that thread: `ParameterText::setValue` is "Any thread" (`jam_ParameterText.h:60`), and `ParameterAttachment` moves the callback to the message thread (`jam_Model.cpp:220-234`).
- The job returns through `callAsync` to `void setFiles (const juce::ValueTree& files, int timeout);`. `setFiles` writes `Id::timeout`, then removes the `FILES` row of the tree with `removeChild` and adds `files` with `appendChild`: the Part 6 form (`PLAN-ling-6-manifest.md:211`), "A complete state replaces a complete state" (MANIFESTO **E**). A `FILES` row is built on the pool thread with no parent and no listener. After the handoff, no thread changes it except the message thread ("Published data is immutable", MANIFESTO **E**).
- The reaction of `LINGModel` to `Id::manifest` keeps only `watcher.addFolder` of the manifest folder (section In LING: the watched manifest).

**C2. No pending request.**

- `Processor::valueTreeChildAdded` runs a `REQUEST` row at once, with no condition. The test `pendingJobs == 0 and model.state.hasProperty (Id::documents)` (`Processor.cpp:478`) is deleted. `setDocuments` keeps only the write of `Id::documents` (`:550-555`). The request scan (`:557-563`) is deleted. This also ends a re-derive of state by a walk (CODING.md, Consume the event payload).
- No client reaches the daemon before the first `Id::documents`, because the port file is written then.
- A reload, a new `FILES` row, erases only the documents whose path has no row in the added row: for each stored document, `childWhichHasBeenAdded.getChildWithProperty (Id::path, paths.at (pathIndex))` is not valid. Every other document stays until its job replaces it with `addOrReplace` (`Processor.cpp:527`), as an edit of a file does in Part 6. `valueTreeChildRemoved` (`Processor.cpp:484-489`) is deleted. Each request thus reads complete documents (MANIFESTO **E**, "Each consumer downstream receives complete data").
- `runRequest` writes `Id::result` inside the add event of the row. `Connection` sends it and removes the row in the same call. So a `REQUEST` row never outlives its add event, and `Server::timerCallback` needs no test.

**C3. The audit resolutions of 2026-11-13.** ARCHITECT answered the two open findings with the CONTRACT files (`carol/MANIFESTO.md`, `carol/CODING.md`, `carol/NAMES.md`).

- **The read of the manifest is its own object, `Manifest`** (audit #31). MANIFESTO **L**: "300 lines per file — too many responsibilities. Split the object." CODING.md: "decompose a multi-responsibility object into objects, and each object is again one header and one `.cpp`." `Processor.cpp` held three jobs: the documents, the read of the manifest (C1), and the nine operations.
  - `class Manifest : private juce::ValueTree::Listener` (`ling/Source/Manifest.h` and `.cpp`) owns `juce::ThreadPool pool` with `threadCount { 1 }`. It reacts to `Id::manifest` on the root with `addJob()`, and it holds `setFiles` and `getTimeout`. It is the C1 code, moved unchanged.
  - The name is `Id::manifest`, in the family of `CompilerManifest`. The members take the family of `Processor` (`pool`, `addJob`).
  - `Processor` keeps the documents and the operations. Neither object calls the other: both react to the model.
  - The application makes `std::unique_ptr<Manifest> manifest` in the manifest run and in the daemon run, after `processor`.
  - Every C1 text above that names `Processor::addManifestJob`, `manifestPool`, `Processor::setFiles`, or `Processor::getTimeout` now names `Manifest::addJob`, `Manifest::pool`, `Manifest::setFiles`, and `Manifest::getTimeout`.
- **The messages of the daemon reach the client** (audit #4). MANIFESTO anti-pattern: "Silent fail — **E** (Explicit)". The daemon's stderr is `/dev/null`, so `model.setMessage` alone reached no one.
  - `Server` holds `jam::Model::ParameterAttachment message` on `Id::message` of the `OVERLAY` row. This is the established path of the application (`Main.cpp:279-281`).
  - Its callback tells each connection of `connections` to send the text as a JSON string (`juce::JSON::toString`).
  - A result is a JSON array. `Client::messageReceived` tests the first byte of the reply against `Chars::doubleQuote`:
    - A string goes to `model.setMessage` of the client. The application of the client then prints `ling: <operation>: <text>` on stderr, as `HELP.md` states.
    - An array is printed on stdout, and the client quits.
  - `Processor` writes the message of a wrong request before its result (`Processor.cpp:257-258`), so the client prints the message, then `[]`.

## Validation Gate

Each step is validated by COUNSELOR before proceeding to the next — against
MANIFESTO.md (BLESSED), NAMES.md, ~/.carol/CODING.md, and the locked PLAN decisions
(no deviation, no scope drift). @Auditor runs ONCE, after the final step, covering
the whole sprint — never per step.

## Steps

### Step 1: `jam::ChildProcess::startDetached`, the run signatures (ruling 52), and `jam::Format::appendHex`
**Status:** Done 2026-11-13, validated by COUNSELOR. Item 7 had a fourth caller: `ling/Source/CompilerManifest.cpp:140` passes `getDependencyArguments`, which now returns `jam::Strings` (`CompilerManifest.h:113`; `CompilerManifest.cpp:157-159`).
**Scope:** `jam/jam_subprocess/subprocess/jam_ChildProcess.h`; `native/jam_ChildProcess_posix.cpp`; `native/jam_ChildProcess_windows.cpp`; `jam/jam_subprocess/subprocess/jam_Subprocess.h` and `.cpp`; `jam/jam_core/text/jam_Strings.h` and `.cpp`; `jam/jam_core/text/jam_Format.h` and `.cpp`; `scout/Source/Toolchain.cpp`; `scout/Source/Benchmark.cpp`; `cast/Source/Toolchain.cpp`.
**Action:**
1. Declare `static bool startDetached (const jam::Strings& arguments);` after `start` in `jam_ChildProcess.h`.
2. POSIX: write two file-static functions. `static jam::Array<char> getArgumentBytes (const jam::Strings& arguments)` returns the bytes of each argument that is not empty, each with one NUL after it: `jam::Strings` holds no NUL for an element (`jam_core/text/jam_Strings.h:16-22`). This is the one copy at the OS boundary (MANIFESTO, Permitted copies, item 1). An empty argument gives no bytes, as the loop of `Native` gives it no pointer (`:22-24`). `static jam::Array<char*> getArgumentPointers (const jam::Array<char>& bytes)` returns one pointer for each argument, then a final `nullptr`: the pointer to the first byte, and the pointer to each byte that follows a NUL and is not the end of `bytes`. Each pointer is a `const_cast<char*>`, as in the loop of `Native` (`:24`), because `execvp` and `posix_spawn` take `char* const*`. Each function returns its value (CODING.md, no out-parameter). The caller holds the bytes in a local for as long as it uses the pointers. `Native` calls both too (item 5). Write `startDetached` as section In JAM gives it. The platform headers `<spawn.h>` and `<fcntl.h>` go at the top of the native file, next to `<sys/wait.h>` (`:3`).
3. Windows: write `startDetached` as section In JAM gives it. Build the command line with `getQuotedArgument`, in the form of `start` (`:157-160`).
4. Done in Part 6 (PLAN-ling-6-manifest.md, In LING: the application; the sort of `applyToDocuments` read a copy per compare). `jam::Strings`: declare the public member `std::string_view getView (int index) const;` after `at` (`jam_core/text/jam_Strings.h:565`), and write its body in `jam_Strings.cpp`. It returns the private `getView (Span)` (`:754`) for the span at `index`. The precondition is the precondition of `at`: `index` is within [0, size()). The view is valid until the container changes. The name joins the `getView` family of the type (`:633`, `:754`; NAMES.md Rule 5). `at` and the iterator make a `juce::String` for each read (`:565`, `:590`); this member makes none.
5. `ChildProcess::start`: the first parameter becomes `const jam::Strings& arguments`, in the header (`jam_ChildProcess.h:44`), in the POSIX body (`jam_ChildProcess_posix.cpp:165`), and in the Windows body (`jam_ChildProcess_windows.cpp:147`). POSIX: the constructor of `Native` (`:12`) takes `const jam::Strings&`; `getArgumentBytes` and `getArgumentPointers` replace its loop (`:20-26`); the executable of `:14` is `arguments.at (0).unquoted()`, a `juce::String` because `juce::File::getChildFile` reads it (`:17`; ruling 50, a JUCE API takes one). Windows: `:151` reads `arguments.at (0)`, and the loop of `:159-160` reads `arguments.at (index)`. `getQuotedArgument` (`:115-145`) and the Windows `Native` (`:8`) keep `juce::String`, because `CreateProcessW` reads the text of `toWideCharPointer` (`:27`), a JUCE conversion.
6. `Subprocess`: `launch` takes `const jam::Strings& command` (`jam_Subprocess.h:95`; `jam_Subprocess.cpp:194`). The `Worker` constructor takes `const jam::Strings& newCommand` (`jam_Subprocess.cpp:32`), and the member `command` is a `jam::Strings` (`:180`). The assert of `:199` becomes `jassert (command.size() > 0)`, the form of the assert in `start` (`jam_ChildProcess_posix.cpp:167`).
7. The callers of `launch`, three in all. `scout/Source/Toolchain.cpp:21` and `cast/Source/Toolchain.cpp:21` pass `jam::Strings { arguments }`, through the constructor `explicit Strings (const juce::StringArray&)` (`jam_Strings.h:124`). `scout/Source/Benchmark.cpp:110` passes a `jam::Strings` of the two path texts, through the variadic constructor (`jam_Strings.h:94-106`), in place of the `juce::StringArray` temporary. `start` has one caller, `Subprocess::Worker::run` (`jam_Subprocess.cpp:64`); it needs no change, because `command` is now a `jam::Strings`.
8. `jam::Format`: declare `static void appendHex (std::string& text, std::uint64_t value);` after `appendDecimal` (`jam_core/text/jam_Format.h:412`), and write its body in `jam_Format.cpp` after the body of `appendDecimal`. It has the form of `appendDecimal`: `std::to_chars` into a stack buffer, here with base 16, then one append to `text`. The digits are lowercase, as `std::to_chars` gives them. It writes no prefix and no padding. It has no assert: every `std::uint64_t` value is valid.

**Validation:** `startDetached` is one function with two native bodies. No new type. `getArgumentBytes` and `getArgumentPointers` are the one argv build of the POSIX file; `Native` and `startDetached` both call them (MANIFESTO **S**). Each returns its value (CODING.md, no out-parameter). `appendHex` joins the `append*` family of `jam::Format` (NAMES.md Rule 5). `start` and `launch` take `jam::Strings` (ruling 52). `Strings::getView (int)` gives an element with no `juce::String`. The only includes are platform headers in the native file (CODING.md). No other member of `ChildProcess` changes. The function holds no handle after it returns (MANIFESTO **B**: the parent does not own the child).

### Step 2: The rows
**Status:** Done 2026-11-13, validated by COUNSELOR. The cast run of item 6 is part of Step 7 item 1: in `ling/`, cast formats, generates, and builds, and "A build is ARCHITECT's" (`PLAN-ling-0-tracker.md:50`).
**Scope:** `ling/cast/files.md`; `ling/cast/text.md`; `ling/cast/identifiers.md`; `ling/cast/c_language.md`.
**Action:**
1. `## user module` already holds the `jam_subprocess` row: Part 6 Step 4 adds it with `jam::ChildProcess::getArguments`.
2. `cast/files.md`: add `@string`, `lingCache`, `` `.cache/ling` ``, `Cache folder of LING below the home folder or a repository root.`; and `@string`, `lingLocalCache`, `` `ling` ``, `Cache folder of LING in %LOCALAPPDATA%.` (ruling 58).
3. `cast/files.md`: add `@string`, `lingPort`, `` `port` ``, `Extension of the port file of a daemon.`
4. `cast/text.md`: add `failDaemonStart` `` `daemon did not start` `` and `failDaemonLost` `` `daemon connection lost` ``. Add `failDaemonTimeout` `` `daemon timeout must be a positive integer` ``, in the form of cast's `failFormatWidth`, `` `format width must be a positive integer` `` (`cast/cast/text.md:55`).
5. `cast/c_language.md`, `## cLanguage`: add the row `dependencyRecord`, `` `.ninja_deps` ``, `The file of the build folder that changes when the dependency record changes.`, after the row `dependencyHead` (PLAN-ling-6-manifest.md Step 4 item 2). The row comes in this part, which holds its first reader.
6. `cast/identifiers.md`: add `compilerManifests`, `dependencyRecords`, `daemon`, `connections`, `connection`, `timeout`, in name order. No `@id` row in a `jam/cast/*.md` file holds `compilerManifests` or `dependencyRecords` (read of 2026-11-11). Run cast (tracker, The contract of every part, cast runs).

**Validation:** Each row has the cell shape of its table. The two cache rows have the shape of cast's cache root (`jam/cast/juce.cast:8-12`; ruling 58). No `## define` row is added. No `project-info.md` gets a row: `ling/project-info.md` is the build manifest of the tool, and the `## daemon` table is an override that a project can hold (rulings 71 and 72).

### Step 3: The read of the manifest off the message thread, and no pending request
**Scope:** `ling/Source/Processor.h` and `.cpp`; `ling/Source/LINGModel.h` and `.cpp`; `ling/Source/Validator.cpp`.
**Action:**
1. `LINGModel`: write `static constexpr int defaultTimeoutMs { 60000 };` and its write to `Id::timeout` in the constructor. The reaction to `Id::manifest` keeps only `watcher.addFolder` of the manifest folder (C1).
2. `Validator::addRules`: add the rule `daemon`, as section The `## daemon` table gives it. `Id::value` exists in `jam/cast/core.md:154`.
3. `Processor`: write C1 (`manifestPool`, `manifestThreadCount`, `addManifestJob`, `getTimeout`, `setFiles`), and C2 (the request runs at once; `setDocuments` writes only `Id::documents`; the erase on a new `FILES` row; `valueTreeChildRemoved` deleted). Delete first, then write (CAROL.md Refactor-Rewrite Discipline).
**Validation:**
- No wait and no request scan on the message thread. The wait for `ninja` runs only on `manifestPool`. The erase of C2 tests each stored document against the new `FILES` row, once for each reload (section Risks).
- Each property has its author of the state tree table. The `FILES` row is complete before `appendChild` (MANIFESTO **E**, Complete at Creation).
- Each reaction consumes its event payload (CODING.md).
- The job takes values only, with no `juce::ValueTree` of the model, as `addJob` does (`Processor.cpp:501`).

### Step 4: The watched manifest
**Status:** Done 2026-11-13, validated by COUNSELOR (`CompilerManifest.cpp:71-77`, `:151-163`; `LINGModel.cpp:27-87`). Step 3 moves the read out of the reaction of `LINGModel`. The `watcher.addFolder` of the manifest folder stays there.
**Scope:** `ling/Source/LINGModel.h` and `.cpp`; `ling/Source/CompilerManifest.cpp`.
**Action:**
1. `CompilerManifest::addCLanguageFiles`: append the full path of the compiler manifest to `Id::compilerManifests` of the `FILES` row, after the test that the file exists (PLAN-ling-6-manifest.md, The C family entry, The units, item 2). Append the full path of the dependency record file to `Id::dependencyRecords` of the row, after a run of the `ninja` row that exits with 0 (The files, item 3).
2. `LINGModel`: add the parent directory of the manifest to the watcher when `Id::manifest` is written. In `valueTreeChildAdded` for the `FILES` row, add the parent directory of each file of `Id::compilerManifests` and of `Id::dependencyRecords`. In `fileChanged`, the file of `Id::manifest` and each file of the two properties of the `FILES` row give `sendPropertyChangeMessage (Id::manifest)` on the root, for any event.
**Validation:** The reload is the Part 6 reaction to `Id::manifest`. No second code path reads the manifest, a compiler manifest, or a dependency record. `LINGModel` reads the paths from the row; it names no file of a language. The file name of the record is a row of `cast/c_language.md`. `fileChanged` dispatches by the file, and not by the event type.

### Step 5: `Server`, `Connection`, and the daemon application
**Scope:** new files `ling/Source/Server.h` and `.cpp`; `ling/Source/Connection.h` and `.cpp`; `ling/Source/LINGModel.h` and `.cpp`; `ling/Source/Main.cpp`; `ling/Source/HELP.md`.
**Action:**
1. `LINGModel`: make the `CONNECTIONS` row in the constructor, in the order of the tree of section In LING: the state tree (PLAN-ling-6-manifest.md, item 1 of `LINGModel`). Write `getDaemonName` and `getCacheDirectory` as section In LING: `Server` and `Connection` gives them.
2. Write `Server` and `Connection` as section In LING: `Server` and `Connection` gives them. `Server.h` includes `Connection.h`. `Connection` takes `jam::Owner<Connection>&` in its constructor.
3. Write `ling daemon <project-info.md> [--<word>]` as section In LING: the daemon application gives it, with its item 7. The manifest is the argument after the word `daemon`, and `getToolchainArgument` (PLAN-ling-6-manifest.md Step 6 item 2) takes the index of the manifest. `initialise` keeps its three paths (`Main.cpp:336-354`): a manifest (Part 6, ruling 70); an existing file after the first argument; any other command line. The second path tests the first argument against the text of `Id::daemon` with `juce::String::compare (...) == 0` (CODING.md: the one text test that is unavoidable), and runs `runDaemon` or `runOperation` (the client, Step 6). `runCommandLine` runs only when no file exists, so a test there would never see a manifest. The run of `ling daemon <project-info.md>` never matches the first path: `isDocument` takes a second argument only when it is a `--<word>` (`Main.cpp:223-227`). The process makes `Processor` only in the manifest run and the daemon run, as `std::unique_ptr<Processor> processor`: a `juce::ThreadPool` starts its threads at construction, and a client run uses none. Describe the command in `HELP.md`.
**Validation:**
- No lock in LING code. The listener thread only constructs a `Connection` and posts its ownership. Every tree touch is in a callback on the message thread (MANIFESTO **B**).
- The `stop()` in the destructor names its threat (The Guard Rule).
- `connectionLost` ends with `connections.remove (connections.indexOf (this))`, and touches nothing after it.
- `Connection` and `Server` react to the row and the property that the event gives (CODING.md, Consume the event payload).
- No getter: the server asks no connection for its state. `Server` reads `Id::timeout` from the tree at each start of the timer, and keeps no copy (MANIFESTO **S**). The end of the daemon is its own decision, in `timerCallback`.
- The lock is taken before the server listens, and the port file is written after the first `Id::documents` (`jam_SharedInstance.h:55-56`). The daemon ends only through `Server::timerCallback`. No count is kept beside the tree. Exit at a lock that is not free is a result return, not a bail-out.

### Step 6: The client
**Scope:** new files `ling/Source/Client.h` and `.cpp`; `ling/Source/Main.cpp`; `ling/Source/HELP.md`.
**Action:**
1. Delete the in-process request path of Part 7 Step 5 item 5 from `Main.cpp`: `addRequest` (`Main.cpp:378-390`), its call (`:341-342`), and the request test in the reaction to `Id::documents` (`:406`).
2. Write `Client` and the operation path of `Main.cpp` as section In LING: the client gives them. `Client` has no timer.
3. Describe in `HELP.md` that an operation starts a daemon when none answers, and that a daemon ends when no request comes for the timeout: 60000 ms by default, or the `timeout` row of a `## daemon` table in the manifest of the project.

**Validation:** The client holds no flag of the daemon state. A connect failure starts a daemon, and the watcher event gives the next try (MANIFESTO **E**, tell, do not ask). The client has no timer: each action is a reaction to one event (ARCHITECT: *"why wait? not event?"*). It asks no object whether it is connected (MANIFESTO **E**, Encapsulation rule 3). The message has no manifest and no toolchain argument (`jam_SharedInstance.h:2-3`), and it carries the working folder of the client (section In LING: the client, item 5).

### Step 7: Build and check, macOS
**Scope:** builds of JAM consumers and LING; runs of `ling`; one scout manifest.
**Action:**
1. ARCHITECT runs `cast cast/spell.md --debug` in `ling/`, which generates the rows of Step 2 and builds LING. ARCHITECT builds END, cast, scout, and ggwp. `jam_subprocess` changed, so every project that has the module builds: scout, cast, and ggwp declare it in `project-info.md`. `jam_core` changed (`Strings::getView`, `Format::appendHex`), so END builds too.
2. Run `ling daemon ~/Documents/Poems/dev/end/project-info.md --debug` (ruling 65: END's manifest, as it is). Check: one port file exists in `~/.cache/ling/` after the parse ends, and none before. A second `ling daemon` with the same manifest and the same toolchain argument exits at once with code 0. A `ling daemon` with the same manifest and no toolchain argument stays, and a second port file exists.
3. With no daemon running, run `goToDefinition` of Part 7 Step 6 item 2 two times, one after the other. The first call starts a daemon. The second call joins it: no second daemon process exists. Compare the two outputs byte for byte, and with the output of Part 7.
4. Run four `goToDefinition` calls at the same time, from four shells. One daemon answers all four.
5. Edit `jam_CodeDocument.h`: add a declaration and save. Run `documentSymbol` on the file. The new declaration is in the result, with no new start of the daemon. Save once with an editor that saves by rename, for example vim with `backupcopy=no`. The result is still correct.
6. Press Ctrl-C in the terminal of the client that started the daemon, while its call waits. The daemon still runs.
7. Make no call for 60 seconds. The daemon ends by the default, and the port file is gone. Then write a file `project-info.md` in a temporary folder with one table, `## daemon`, and the row `timeout`, `5000`. Start the daemon with `ling daemon` and that file as the manifest. The file holds no `## toolchain` table, so the daemon holds no document. Make no call for 5 seconds. The daemon ends, and the port file is gone. Then set the `timeout` cell of that file to `abc`, and start the daemon in the same way: its terminal shows the location and the `failDaemonTimeout` text, and `Id::timeout` holds the default. Change no row of `end/project-info.md`.
8. Kill the daemon with `kill -9`. Run an operation. A new daemon starts, and the operation answers.
9. Measure with scout: one benchmark for the start of the daemon to the port file, and one each for `goToDefinition` and `findReferences` against a daemon whose parse has ended. Record the medians beside the medians of Part 7 Step 6 item 5.
10. With the daemon of item 2 live, add a new file `end/Source/Probe.h` with one declaration, and include it in `end/Source/Main.cpp`. ARCHITECT builds END Debug. After the build, run `documentSymbol` on `Probe.h`: the declaration is in the result, with no new start of the daemon. Record the count of reloads that the one build gave, from the count of `Id::documents` events. Then remove the file and the include, and build again.
11. During the build of item 10, run `goToDefinition` in a loop. Each answer comes at once, with no `failDaemonLost`.

**Validation:** Items 2 to 8, 10, and 11 hold. Item 3: the outputs are equal (MANIFESTO **D**). Item 9: the medians are recorded. ARCHITECT reads the timings against RFC ruling 4, "ULTRA FAST" (tracker Canon, From the removed RFC).

### Step 8: Build and check, Windows
**Scope:** a Windows machine, MSYS2 zsh.
**Owner:** ARCHITECT builds, runs, and debugs on the Windows machine (ruling 78). This step blocks no other step and no other part.
**Action:**
1. Build LING.
2. Run items 2 to 8 and item 10 of Step 7 on the Windows machine. In item 6, press Ctrl-C in the MSYS2 zsh terminal. In item 8, end the daemon with `taskkill /F`.
3. Start an operation from nvim on Windows, then end nvim at once. Record whether the daemon still runs until its timer fires.

**Validation:** The results equal the results of macOS (ruling 16). Item 3 checks `CREATE_BREAKAWAY_FROM_JOB`: `CreateProcessW` fails with that flag when the job of the client does not permit a breakaway. A failure is `failDaemonStart`, and it is recorded for ARCHITECT.

## BLESSED Alignment

- **B — Bound.** `Server` owns the connections and the lifetime of the daemon. The lifetime is traceable: it starts at the lock and ends in `timerCallback`. A connection owns its row. The listener thread only constructs an object and posts its ownership to the message thread; every other touch is on the message thread.
- **L — Lean.** Three new JAM members: `ChildProcess::startDetached`, `Strings::getView (int)`, and `Format::appendHex`. Three LING objects: `Server`, `Connection`, and `Client`, and two command paths in `Main.cpp`. The transport is JUCE's.
- **E — Explicit.** A failed start and a lost connection are messages with an exit code. The port file and the lock are files with names that the manifest and the toolchain argument give.
- **S — SSOT.** The documents stay in one process, so they are not parsed again for each call. The length of the timer is one property, read from the tree at each start; its default is one constant. A request is a row that lives only inside its add event; nothing keeps a count. One daemon for one manifest and one toolchain argument, by the lock.
- **S — Stateless.** The client keeps no flag of the daemon and no timer. It connects, or it starts a daemon and waits for the event. The daemon decides its own end (ruling 48).
- **Lock-free, no wait.** No LING code takes a lock. The ownership of a connection moves to the message thread through `callAsync` (MANIFESTO, Permitted copies, item 2). The wait for `ninja` runs on `manifestPool`, never on the message thread. No request waits: each is answered inside its add event.
- **E — Encapsulation.** Tell, never ask: a connection writes a row, `Processor` writes a property, the connection sends it. The server never asks a connection.
- **D — Deterministic.** Each race ends in one daemon (section Why each race ends in one daemon). Step 7 item 3 proves equal output for equal input.
- LANGUAGE.md: no override applies beyond the C++ / JUCE Lean adaptation.

## Risks / Open Questions

None open. Ruling 77 closed tracker Risks item 19.

Facts for the review of the whole PLAN:

- **No `## daemon` table, or no `timeout` row, means the default**, 60000 ms (rulings 53, 71, and 79). It is not a failure, and it gives no message.
- **A `timeout` value that is not an integer greater than zero gives the `failDaemonTimeout` message and the default** (ruling 79). No table of that manifest is read, so the daemon then holds no document.
- **A rebuilt `ling` does not replace a live daemon.** A daemon runs the binary it started with until its timer fires.
- **A change of the manifest reaches a live daemon.** The watcher gives the event, and the manifest job of `Processor` runs again. It reads the `## daemon` table, and `setFiles` writes `Id::timeout` and replaces the `FILES` row.
- **A change of the manifest or of a compiler manifest parses every file again.** `setFiles` replaces the `FILES` row, and `Processor` starts every job again. It erases only the documents whose path has no row in the new row (C2).
- **A request during a reload reads the old document of each file whose job has not ended.** A new file of the manifest appears when its job ends. Each document is complete.
- **The erase of C2 tests each stored document against the new row** with `getChildWithProperty`. Its cost is the count of documents times the count of rows, once for each reload.
- **The destructor of `manifestPool` waits for a running `ninja -t deps`** (`juce_ThreadPool.h:248-253`).
- **`connectToSocket` is synchronous in JUCE** (`juce_InterprocessConnection.cpp:116`). The client calls it once for each port file event, with `connectTimeoutMs`.
- **A message of the daemon reaches every client that is connected at that time** (C3). A request and its message run in one call on the message thread, so another client is connected at that moment only between its connect and its own request.
- **`sendMessage` is synchronous in JUCE.** `Connection` writes each result to the socket on the message thread of the daemon. JUCE gives no asynchronous send.
- **`juce::Identifier` of a command word takes the lock of JUCE's string pool** (`Processor.cpp:251`, Part 7). It is a JUCE internal lock, as the locks inside `InterprocessConnection` are.
- **A daemon that dies before it writes its port file leaves the client waiting** until Ctrl-C. The client has no timer (ARCHITECT: *"why wait? not event?"*).
- **A build reaches a live daemon** (ruling 77). ninja appends to `.ninja_deps` as each compile ends (`deps_log.h:65-67`), so one build can give more than one reload. The watcher coalesces the events of one file inside `coalesceMs`. Each reload replaces the `FILES` row and parses every file again; Part 11 reads an unchanged file from its swap. Step 7 item 10 counts the reloads of one build.
- **A reload during a build reads the records that exist at that time.** The last event of the build gives the last reload, with every record.
- **A run of `ninja -t deps` while ninja builds is not verified.** Step 7 item 10 is its first run.
- **A compiler manifest or a record file that comes after the start is not watched.** `Id::compilerManifests` holds only the files that a read found on disk. A build that writes its first `compile_commands.json` later gives rows at the next change of the manifest, or in a new daemon.
- **One manifest path and one toolchain argument, one daemon.** Two spellings of one path give two names. On Windows, a path with another letter case is another daemon.
- **Every build of `ling` shares `~/.cache/ling/`** (ruling 58). A Debug build and a Release build of `ling` give one manifest and one toolchain argument the same daemon name, the same lock, and the same port file. The build that starts first serves both.
- **The watcher is recursive on macOS and Windows** (`jam_Listener.h:38-41`). The client watches `~/.cache/ling/` and its children. The folder also holds the precompiled headers of Part 10, and the swap files of Part 11 that have no root. The client reacts only to its port file.
- **The default of the timer** is 60000 ms, by ruling 53.
- **The stdio LSP wrapper is closed** (tracker Risks item 16). No part holds it.
- **The callers of the run API, as read on 2026-11-11.** A read of 2026-11-13 found a fourth call: `ling/Source/CompilerManifest.cpp:140` (Step 1 Status). The LSP gives three calls of `launch`: `scout/Source/Toolchain.cpp:21`, `scout/Source/Benchmark.cpp:110`, `cast/Source/Toolchain.cpp:21`. For ggwp, and for `ChildProcess::start`, the LSP gave no answer; a text search found no call. A text search does not prove absence. The build of Step 7 item 1 is the proof.
- **The `juce::StringArray` before the call stays.** `Toolchain::getToolchainArguments` and `Toolchain::runProcess` (scout and cast `Toolchain.cpp:3`, `:15`) and `Pack::getImageArguments` (`cast/Source/Pack.cpp:158`) build and pass a `juce::StringArray`. Step 1 item 7 converts it at the `launch` call: one copy of the arguments for each run. Ruling 52 names the callers; it does not name these three functions.
