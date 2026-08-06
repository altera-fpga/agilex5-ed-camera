# Settings for ubifs
MKUBIFS_ARGS ?= "--leb-size ${UBIFS_LEB_SIZE} --min-io-size ${PAGE_SIZE} --max-leb-cnt ${UBIFS_LEB_CNT}"
UBINIZE_ARGS ?= "--peb-size ${UBIFS_PEB_SIZE}KiB --min-io-size ${PAGE_SIZE} --sub-page-size ${PAGE_SIZE}"
MULTIUBI_BUILD += "nand"
MKUBIFS_ARGS_nand ?= "--leb-size ${UBIFS_LEB_SIZE} --min-io-size ${PAGE_SIZE} --max-leb-cnt ${UBIFS_LEB_CNT}"
UBINIZE_ARGS_nand ?= "--peb-size ${UBIFS_PEB_SIZE}KiB --min-io-size ${PAGE_SIZE} --sub-page-size ${PAGE_SIZE}"
MULTIUBI_BUILD += "nor"
MKUBIFS_ARGS_nor ?= "-F -e 65408 -m 1 -c ${UBIFS_LEB_CNT}"
UBINIZE_ARGS_nor ?= "-p 65536 -m 1 -s 1"

