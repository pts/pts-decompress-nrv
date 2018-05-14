pts-decompress-nrv: educational C code for NRV decompression

pts-decompress-nrv is education C implemenetation of NRV{8,16,32}{B,D,E}
decompression. The compression algorithm is used by UPX
(https://upx.github.io/) to create small compressed executables with a tiny
(few hundred bytes of hand-optimized machine code) decompressor code.

The NRV compression formats lack publicly available documentation, and the C
source code in the reference implementation
(http://www.oberhumer.com/opensource/ucl/download/ucl-1.03.tar.gz) is quite
hard to understand. pts-decompress-nrv is an easier-to-understand
alternative implementation which can be used for education.

Ideally pts-decompress-nrv should contain more comments and documentation.

__END__
