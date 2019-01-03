# OneDriveFS

A FUSE driver for Microsoft OneDrive™.

## Usage

Before you can mount your personal cloud drive:

* create a `config.json` file in `~/.config/onedrivefs` by copying `config.json.template`
* set the _client ID_ (also known as _application ID_) and _authorization code_
  * the _client ID_ is obtained by registering the application [here](https://apps.dev.microsoft.com/)
  * the _authorization code_ is obtained by opening a link like the following in your browser (notice the _client_id_ token):

        https://login.microsoftonline.com/common/oauth2/v2.0/authorize?client_id=${client_id}&scope=files.readwrite.all%20offline_access&response_type=code&redirect_uri=https%3A%2F%2Flogin.microsoftonline.com%2Fcommon%2Foauth2%2Fnativeclient

    After signing in, look in the address bar for a string that looks something like `Mcb1aa9bf-b777-911b-2719-fddeacda4713`. Copy it in your `config.json`.

Once all the needed information has been collected and set, you can do:

    $ onedrivefs <your-mount-point>

and if you want to remove the mount:

    fusermount -u <your-mount-point>

## Features

* List the drive contents
* Get the drive available space and total size
* Read files
* Delete files
* Delete directories
* Get the SHA1 hash of each file (if available) via xattr-s (see `getfattr -d <file>`)
* Basic directory entry cache

## Building

You will need:

* gcc >= 4.8.1
* libcurl >= 7.63
* jsoncpp >= 1.8
* fuse >= 2.9
* meson
* ninja

In order to build the executable do:

    $ git clone https://github.com/mdontu/onedrivefs.git
    $ cd onedrivefs
    $ mkdir build
    $ cd build
    $ meson ..
    $ ninja

## Known Issues

* In-application OAuth2 is not supported (hence the dance with the _client ID_ and _authorization code_)
* Only the root drive is exposed
* Writes are not supported
* Copying files is not very fast. Use `dd` with a large block size (1MiB or more) as each `read()` translates into an HTTPS request
* Deleting files moves them to Recycle Bin

## Resources

* [Microsoft Graph](https://graph.microsoft.com/)
* [Microsoft OneDrive REST API](https://docs.microsoft.com/en-us/onedrive/developer/rest-api/)
* [Microsoft Application Registration Portal](https://apps.dev.microsoft.com/)
* [libfuse](https://github.com/libfuse/libfuse/)
