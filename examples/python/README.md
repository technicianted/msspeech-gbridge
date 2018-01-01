## Using Google Python client libraries

References and source [here](https://cloud.google.com/speech/docs/reference/libraries#client-libraries-usage-python)

Use unmodified Google Speech APIs to access Microsoft Cognitive Services Speech APIs.

### Changes

All that is need to to pass a `grpc.insecure_channel` object with `msspeech-gbridge` endpoint to `speech.SpeechClient`. Below is an example of [`transcribe.py`](https://github.com/GoogleCloudPlatform/python-docs-samples/blob/master/speech/cloud-client/transcribe.py):

```patch
27a28,29
> import grpc
> import sys
32c34
< def transcribe_file(speech_file):
---
> def transcribe_file(speech_file, chan=None):
37c39
<     client = speech.SpeechClient()
---
>     client = speech.SpeechClient(channel=chan)
64c66
< def transcribe_gcs(gcs_uri):
---
> def transcribe_gcs(gcs_uri, chan=None):
69c71
<     client = speech.SpeechClient()
---
>     client = speech.SpeechClient(channel=chan)
93a96,97
>     parser.add_argument(
>         '--gbridge-endpoint', dest="endpoint", help='Optional endpoint to use for msspeech-gbridge service')
94a99,103
> 
>     chan = None
>     if args.endpoint != None:
>         chan = grpc.insecure_channel(args.endpoint)
> 
96c105
<         transcribe_gcs(args.path)
---
>         transcribe_gcs(args.path, chan)
98c107
<         transcribe_file(args.path)
---
>         transcribe_file(args.path, chan)
```

#### Google Speech APIs

Invoke as documented (no command line arguments). For example:

```
./transcribe.py <path to audio file>
```

#### Microsoft Speech APIs

Add a command line argument with `--gbridge-endpoint` endpoint. For example:

```
./transcribe.py --gbridge-endpoint localhost:8080 <path to audio file>
```

