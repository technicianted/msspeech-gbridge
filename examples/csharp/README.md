## Using Google C# client libraries

References and source [here](https://cloud.google.com/speech/docs/reference/libraries#client-libraries-usage-csharp)

Use unmodified Google Speech APIs to access Microsoft Cognitive Services Speech APIs.

Note that this directory contains dotnet core 2.0 application. But it is running the same [code](https://github.com/GoogleCloudPlatform/dotnet-docs-samples/tree/master/speech/api/QuickStart) from Google's repository.

### Changes

You will need to create `SpeechClient` with `Channel` pointing to `msspeech-bridge`:

```c#
    speech = SpeechClient.Create(new Channel(endpoint, ChannelCredentials.Insecure));
```
where `endpoint` is something like `localhost:8080`.

#### Google Speech APIs

Invoke as documented (no command line arguments). For example:

```
dotnet run <audio file>
```

#### Microsoft Speech APIs

Add a command line argument with `msspeech-gbridge` endpoint. For example:

```
dotnet run <audio file> localhost:8080
```

