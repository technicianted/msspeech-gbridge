References and source [here](https://cloud.google.com/speech/docs/reference/libraries#client-libraries-install-go)

Use unmodified Google Speech APIs to access Microsoft Cognitive Services Speech APIs.

### Changes

This example changes the sample [`livecaption.go`](https://github.com/GoogleCloudPlatform/golang-samples/blob/master/speech/livecaption/livecaption.go) to be able to connect to both Google APIs and `msspeech-gbridge`.

Change `speech.NewClient` invokation to pass `msspeech-gbridge` a custom `grpc.ClientChann` to point to `msspeech-gbridge` endpoint:

```patch
10a11
> 
20a22
> 	"google.golang.org/api/option"
21a24
> 	"google.golang.org/grpc"
26a30,37
> 	var options []option.ClientOption
> 	if len(os.Args) > 1 {
> 		conn, err := grpc.Dial(os.Args[1], grpc.WithInsecure())
> 		if err != nil {
> 			log.Fatal(err)
> 		}
> 		options = append(options, option.WithGRPCConn(conn));
> 	}
28c39
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

