/*
 * Copyright (c) 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

// [START speech_quickstart]

using Google.Cloud.Speech.V1;
using System;
using Google.Api.Gax.Grpc;
using Grpc.Core;

namespace GoogleCloudSamples
{
    public class QuickStart
    {
        public static void Main(string[] args)
        {
            SpeechClient speech = null;
            if (args.Length == 1) 
            {
                // use default Google API
                Console.WriteLine("Using Google APIs");
                speech = SpeechClient.Create();
            }
            else
            {
                // use msspeech-gbridge
                Console.WriteLine("Using msspeech-bridge at " + args[1]);
                speech = SpeechClient.Create(new Channel(args[1], ChannelCredentials.Insecure));
            }

            var response = speech.Recognize(new RecognitionConfig()
            {
                Encoding = RecognitionConfig.Types.AudioEncoding.Linear16,
                SampleRateHertz = 16000,
                LanguageCode = "en-US",
            }, RecognitionAudio.FromFile(args[0]));
            foreach (var result in response.Results)
            {
                foreach (var alternative in result.Alternatives)
                {
                    Console.WriteLine(alternative.Transcript);
                }
            }
        }
    }
}
// [END speech_quickstart]
