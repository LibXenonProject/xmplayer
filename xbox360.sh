ftp -v -n "192.168.1.111" << cmd
user "xbox" "xbox"
bin
delete /Usb0/xenon.elf
delete /Usb0/symbols.elf
put xmplayer.elf32 /Usb0/xenon.elf
put xmplayer.elf /Usb0/symbols.elf
dis
quit
cmd
