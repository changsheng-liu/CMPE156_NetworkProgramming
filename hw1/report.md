# Report

## Usage

After `make`, run `./httpdownloader param1 param2 (param3)`. Param3 is optional.
Param1 has to be a valid IP address.
Param2 has to be a valid url, which includes host and target. Port number in Param2 is optional.
Param3 has to be `-h` to call HTTP HEAD or nothing for HTTP GET.

## Function Description

This program has two functions. One is main function, the other is to verify if the IP address is valid.
In Verify IP function, I first simple verify ip address by regex, then check if it is a valid IP by `sockaddr_in`.
In main function, first I process the input, the buid a socket and connect to the server as instruction in the class, and read the response as instruction in the class at last.
Most of my work is on preprocess input. I need to check if the input is a valid arguement format, a valid ip address, a valid host, port and target.
After I get the server response, I read the response by a buffer size, and either print directly to screen or save into a file so that I could not be bothered by memory problem. 
**extra:** I read the `output.dat` file again in the end line by line, and check if there is a `\r\n` at first time. This mark is the mark of the end of HTTP head. I remove all above and save the below in `output2.dat`.
**extra2:** I set up a 4 minutes timer to fire in case the read tcp never ends.

## Shortcome

1. Input invalid may not cover every typo.

2. If the http response is a binary file, it may not restore as itself.

3. If the ip address is legal but not valid, connect function may stall, like 222.222.222.222
