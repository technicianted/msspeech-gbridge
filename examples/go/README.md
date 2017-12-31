References and source [here](https://cloud.google.com/speech/docs/reference/libraries#client-libraries-install-go)

Use unmodified Google Speech APIs to access Microsoft Cognitive Services Speech APIs.

### Changes

Change `speech.NewClient` invokation to pass `msspeech-gbridge` service endpoint and disable authentication:

```patch
29a30,33
> 	var options []option.ClientOption
> 	if len(os.Args) > 1 {
> 		options = append(options, option.WithEndpoint(os.Args[1]), option.WithoutAuthentication(), option.WithGRPCDialOption(grpc.WithInsecure()))
> 	}
31c35
< 	client, err := speech.NewClient(ctx)
---
> 	client, err := speech.NewClient(ctx, options...)
```

### Usage

Follow instructions [here](https://cloud.google.com/speech/docs/reference/libraries#client-libraries-install-go) to install Google Speech client Go libraries.

#### Google Speech APIs

Invoke as documented (no command line arguments). For example:

```
cat myaudiofile.wav | go run stream.go
```

#### Microsoft Speech APIs

Add a command line argument with `msspeech-gbridge` endpoint. For example:

```
cat myaudiofile.wav | go run stream.go localhost:8080
```

