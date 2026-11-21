#include "StdioClientProbe.h"

#if LING_USING_SCHEMA
juce::Result StdioClientProbe::run (const jam::MarkdownDocument& manifest, const juce::File& folder)
{
    static const jam::HashMap<juce::String, int> streamFlags {
        { jam::Strings::fromView (standardOutput), jam::StdioClient::wantStdOut },
        { jam::Strings::fromView (joinedOutput), jam::StdioClient::wantStdOut | jam::StdioClient::wantStdErr }
    };

    for (const auto* row : manifest.getTableRows (Id::stdioClient))
    {
        jam::Strings arguments (jam::ChildProcess::getArguments (jam::Strings::getView (manifest.getTableValue (*row, Id::flag))));

        arguments.insert (0, manifest.getTableValue (*row, Id::command));

        juce::WaitableEvent finished;
        int byteCount { 0 };
        int exitCode { EXIT_FAILURE };
        jam::StdioClient client;

        client.onBytesReady = [&client, &byteCount] (int)
        {
            client.applyPendingData ([&byteCount] (const char*, int size)
            {
                byteCount += size;
                return size;
            });
        };
        client.onExit = [&finished, &exitCode] (int code)
        {
            exitCode = code;
            finished.signal();
        };

        if (client.open (arguments, folder, streamFlags.at (manifest.getTableValue (*row, Id::streams)), nullptr, bufferSize, maximumPeriodMilliseconds))
        {
            const auto input { manifest.getTableValue (*row, Id::input) };

            if (input.isNotEmpty())
            {
                client.write (input.toRawUTF8(), input.getNumBytesAsUTF8());
                client.flush();
            }

            finished.wait();
        }

        std::string rows;
        rows.append (jam::Strings::getView (manifest.getTableValue (*row, Id::name)));
        rows.append (lineSeparator);
        rows.append (bytesLabel);
        rows.append (jam::Strings::getView (juce::String (byteCount)));
        rows.append (exitLabel);
        rows.append (jam::Strings::getView (juce::String (exitCode)));
        jam::Format::appendUtf8 (rows, Chars::newline);
        fwrite (rows.data(), sizeof (char), rows.size(), stdout);
        fflush (stdout);
    }

    return juce::Result::ok();
}
#endif
