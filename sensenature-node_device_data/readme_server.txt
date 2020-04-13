# upload file form command line: 4.1 is the default ip in AP mode, 
# replace with ip after connecting to network
curl -F "file=@your_file.xyz" 192.168.4.1/edit

# if desired, upload web-editor, will be available at ip/edit in browser
curl -F "file=@edit.htm" 192.168.4.1/edit
curl -F "file=@ace.js" 192.168.4.1/edit

# wifi settings:
# fill out wifi.txt, no quotation marks
# upload to esp and reboot

# show current time:
/get_time

# update time from ntp, defaults to pool.ntp.org
/ntp_update
/ntp_update?server=___

# format spiffs:
/format?sure=yes

# list files:
/list?dir=/

# download file:
/file_path

# file size:
/file_size?file=___

# free space:
/free_space

# delete file:
/delete?file_path

# update firmware:
# 1: upload firmware
# 2: run this, fw file will be deleted at the end
/update?file=file_path




