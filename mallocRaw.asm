0000000000073900 <ggggc_mallocRaw>:
   73900:	41 57                	push   %r15
   73902:	41 56                	push   %r14
   73904:	41 55                	push   %r13
   73906:	41 54                	push   %r12
   73908:	55                   	push   %rbp
   73909:	53                   	push   %rbx
   7390a:	48 83 ec 38          	sub    $0x38,%rsp
   7390e:	64 48 8b 04 25 28 00 	mov    %fs:0x28,%rax
   73915:	00 00 
   73917:	48 89 44 24 28       	mov    %rax,0x28(%rsp)
   7391c:	31 c0                	xor    %eax,%eax
   7391e:	48 81 fe fd ff 1f 00 	cmp    $0x1ffffd,%rsi
   73925:	0f 87 55 03 00 00    	ja     73c80 <ggggc_mallocRaw+0x380>
   7392b:	4c 8b 15 ee b2 20 00 	mov    0x20b2ee(%rip),%r10        # 27ec20 <currentPool>
   73932:	48 89 fd             	mov    %rdi,%rbp
   73935:	48 89 f3             	mov    %rsi,%rbx
   73938:	4d 85 d2             	test   %r10,%r10
   7393b:	0f 84 cf 01 00 00    	je     73b10 <ggggc_mallocRaw+0x210>
   73941:	4d 8b 62 10          	mov    0x10(%r10),%r12
   73945:	41 f6 c4 07          	test   $0x7,%r12b
   73949:	0f 85 b6 00 00 00    	jne    73a05 <ggggc_mallocRaw+0x105>
   7394f:	4c 8d 2c dd 00 00 00 	lea    0x0(,%rbx,8),%r13
   73956:	00 
   73957:	49 83 fd 0f          	cmp    $0xf,%r13
   7395b:	0f 86 af 00 00 00    	jbe    73a10 <ggggc_mallocRaw+0x110>
   73961:	45 31 f6             	xor    %r14d,%r14d
   73964:	45 31 ff             	xor    %r15d,%r15d
   73967:	4b 8d 04 2c          	lea    (%r12,%r13,1),%rax
   7396b:	49 8d 92 00 00 00 01 	lea    0x1000000(%r10),%rdx
   73972:	48 39 d0             	cmp    %rdx,%rax
   73975:	0f 86 19 03 00 00    	jbe    73c94 <ggggc_mallocRaw+0x394>
   7397b:	4d 8b 42 08          	mov    0x8(%r10),%r8
   7397f:	4d 85 c0             	test   %r8,%r8
   73982:	74 60                	je     739e4 <ggggc_mallocRaw+0xe4>
   73984:	49 8b 38             	mov    (%r8),%rdi
   73987:	40 f6 c7 02          	test   $0x2,%dil
   7398b:	0f 84 24 03 00 00    	je     73cb5 <ggggc_mallocRaw+0x3b5>
   73991:	49 8b 40 08          	mov    0x8(%r8),%rax
   73995:	48 39 d8             	cmp    %rbx,%rax
   73998:	0f 84 37 02 00 00    	je     73bd5 <ggggc_mallocRaw+0x2d5>
   7399e:	4c 8d 4b 02          	lea    0x2(%rbx),%r9
   739a2:	4c 39 c8             	cmp    %r9,%rax
   739a5:	72 2e                	jb     739d5 <ggggc_mallocRaw+0xd5>
   739a7:	e9 47 02 00 00       	jmpq   73bf3 <ggggc_mallocRaw+0x2f3>
   739ac:	0f 1f 40 00          	nopl   0x0(%rax)
   739b0:	48 8b 30             	mov    (%rax),%rsi
   739b3:	40 f6 c6 02          	test   $0x2,%sil
   739b7:	0f 84 f8 02 00 00    	je     73cb5 <ggggc_mallocRaw+0x3b5>
   739bd:	48 8b 48 08          	mov    0x8(%rax),%rcx
   739c1:	48 39 d9             	cmp    %rbx,%rcx
   739c4:	74 5a                	je     73a20 <ggggc_mallocRaw+0x120>
   739c6:	49 39 c9             	cmp    %rcx,%r9
   739c9:	0f 86 e9 00 00 00    	jbe    73ab8 <ggggc_mallocRaw+0x1b8>
   739cf:	48 89 f7             	mov    %rsi,%rdi
   739d2:	49 89 c0             	mov    %rax,%r8
   739d5:	48 89 f8             	mov    %rdi,%rax
   739d8:	48 83 e0 fc          	and    $0xfffffffffffffffc,%rax
   739dc:	48 85 c0             	test   %rax,%rax
   739df:	49 89 c4             	mov    %rax,%r12
   739e2:	75 cc                	jne    739b0 <ggggc_mallocRaw+0xb0>
   739e4:	4d 8b 12             	mov    (%r10),%r10
   739e7:	4d 85 d2             	test   %r10,%r10
   739ea:	0f 84 40 01 00 00    	je     73b30 <ggggc_mallocRaw+0x230>
   739f0:	4c 89 15 29 b2 20 00 	mov    %r10,0x20b229(%rip)        # 27ec20 <currentPool>
   739f7:	4d 8b 62 10          	mov    0x10(%r10),%r12
   739fb:	41 f6 c4 07          	test   $0x7,%r12b
   739ff:	0f 84 62 ff ff ff    	je     73967 <ggggc_mallocRaw+0x67>
   73a05:	e8 d6 08 f9 ff       	callq  42e0 <assertPtrAligned.part.3>
   73a0a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
   73a10:	41 bd 10 00 00 00    	mov    $0x10,%r13d
   73a16:	bb 02 00 00 00       	mov    $0x2,%ebx
   73a1b:	e9 41 ff ff ff       	jmpq   73961 <ggggc_mallocRaw+0x61>
   73a20:	83 e7 02             	and    $0x2,%edi
   73a23:	0f 84 b5 02 00 00    	je     73cde <ggggc_mallocRaw+0x3de>
   73a29:	49 89 30             	mov    %rsi,(%r8)
   73a2c:	49 8d 55 f8          	lea    -0x8(%r13),%rdx
   73a30:	49 8d 7c 24 08       	lea    0x8(%r12),%rdi
   73a35:	31 f6                	xor    %esi,%esi
   73a37:	49 c7 04 24 00 00 00 	movq   $0x0,(%r12)
   73a3e:	00 
   73a3f:	e8 bc 07 f9 ff       	callq  4200 <memset@plt>
   73a44:	41 f6 c4 07          	test   $0x7,%r12b
   73a48:	75 bb                	jne    73a05 <ggggc_mallocRaw+0x105>
   73a4a:	48 03 1d bf b1 20 00 	add    0x20b1bf(%rip),%rbx        # 27ec10 <allocated>
   73a51:	48 85 db             	test   %rbx,%rbx
   73a54:	48 89 1d b5 b1 20 00 	mov    %rbx,0x20b1b5(%rip)        # 27ec10 <allocated>
   73a5b:	0f 88 8f 00 00 00    	js     73af0 <ggggc_mallocRaw+0x1f0>
   73a61:	66 0f ef c0          	pxor   %xmm0,%xmm0
   73a65:	f2 48 0f 2a c3       	cvtsi2sd %rbx,%xmm0
   73a6a:	48 8b 05 97 b1 20 00 	mov    0x20b197(%rip),%rax        # 27ec08 <available>
   73a71:	48 85 c0             	test   %rax,%rax
   73a74:	78 5a                	js     73ad0 <ggggc_mallocRaw+0x1d0>
   73a76:	66 0f ef c9          	pxor   %xmm1,%xmm1
   73a7a:	f2 48 0f 2a c8       	cvtsi2sd %rax,%xmm1
   73a7f:	f2 0f 5e c1          	divsd  %xmm1,%xmm0
   73a83:	f2 0f 11 05 75 b1 20 	movsd  %xmm0,0x20b175(%rip)        # 27ec00 <loadFactor>
   73a8a:	00 
   73a8b:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
   73a90:	64 48 33 14 25 28 00 	xor    %fs:0x28,%rdx
   73a97:	00 00 
   73a99:	4c 89 e0             	mov    %r12,%rax
   73a9c:	0f 85 4d 02 00 00    	jne    73cef <ggggc_mallocRaw+0x3ef>
   73aa2:	48 83 c4 38          	add    $0x38,%rsp
   73aa6:	5b                   	pop    %rbx
   73aa7:	5d                   	pop    %rbp
   73aa8:	41 5c                	pop    %r12
   73aaa:	41 5d                	pop    %r13
   73aac:	41 5e                	pop    %r14
   73aae:	41 5f                	pop    %r15
   73ab0:	c3                   	retq   
   73ab1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
   73ab8:	4b 8d 04 2c          	lea    (%r12,%r13,1),%rax
   73abc:	48 29 d9             	sub    %rbx,%rcx
   73abf:	48 89 30             	mov    %rsi,(%rax)
   73ac2:	48 89 48 08          	mov    %rcx,0x8(%rax)
   73ac6:	49 83 08 02          	orq    $0x2,(%r8)
   73aca:	e9 5d ff ff ff       	jmpq   73a2c <ggggc_mallocRaw+0x12c>
   73acf:	90                   	nop
   73ad0:	48 89 c2             	mov    %rax,%rdx
   73ad3:	66 0f ef c9          	pxor   %xmm1,%xmm1
   73ad7:	48 d1 ea             	shr    %rdx
   73ada:	83 e0 01             	and    $0x1,%eax
   73add:	48 09 c2             	or     %rax,%rdx
   73ae0:	f2 48 0f 2a ca       	cvtsi2sd %rdx,%xmm1
   73ae5:	f2 0f 58 c9          	addsd  %xmm1,%xmm1
   73ae9:	eb 94                	jmp    73a7f <ggggc_mallocRaw+0x17f>
   73aeb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
   73af0:	48 89 d8             	mov    %rbx,%rax
   73af3:	66 0f ef c0          	pxor   %xmm0,%xmm0
   73af7:	48 d1 e8             	shr    %rax
   73afa:	83 e3 01             	and    $0x1,%ebx
   73afd:	48 09 d8             	or     %rbx,%rax
   73b00:	f2 48 0f 2a c0       	cvtsi2sd %rax,%xmm0
   73b05:	f2 0f 58 c0          	addsd  %xmm0,%xmm0
   73b09:	e9 5c ff ff ff       	jmpq   73a6a <ggggc_mallocRaw+0x16a>
   73b0e:	66 90                	xchg   %ax,%ax
   73b10:	4c 8b 15 11 b1 20 00 	mov    0x20b111(%rip),%r10        # 27ec28 <poolList>
   73b17:	4d 85 d2             	test   %r10,%r10
   73b1a:	0f 84 f1 00 00 00    	je     73c11 <ggggc_mallocRaw+0x311>
   73b20:	4c 89 15 f9 b0 20 00 	mov    %r10,0x20b0f9(%rip)        # 27ec20 <currentPool>
   73b27:	e9 15 fe ff ff       	jmpq   73941 <ggggc_mallocRaw+0x41>
   73b2c:	0f 1f 40 00          	nopl   0x0(%rax)
   73b30:	45 85 ff             	test   %r15d,%r15d
   73b33:	0f 85 ef 00 00 00    	jne    73c28 <ggggc_mallocRaw+0x328>
   73b39:	49 c7 c4 f8 ff ff ff 	mov    $0xfffffffffffffff8,%r12
   73b40:	48 c7 44 24 08 01 00 	movq   $0x1,0x8(%rsp)
   73b47:	00 00 
   73b49:	48 89 6c 24 10       	mov    %rbp,0x10(%rsp)
   73b4e:	48 c7 44 24 18 00 00 	movq   $0x0,0x18(%rsp)
   73b55:	00 00 
   73b57:	64 49 8b 04 24       	mov    %fs:(%r12),%rax
   73b5c:	48 89 04 24          	mov    %rax,(%rsp)
   73b60:	48 89 e0             	mov    %rsp,%rax
   73b63:	64 49 89 04 24       	mov    %rax,%fs:(%r12)
   73b68:	8b 05 82 b1 20 00    	mov    0x20b182(%rip),%eax        # 27ecf0 <ggggc_stopTheWorld>
   73b6e:	85 c0                	test   %eax,%eax
   73b70:	74 05                	je     73b77 <ggggc_mallocRaw+0x277>
   73b72:	e8 d9 fb ff ff       	callq  73750 <ggggc_yield>
   73b77:	31 ff                	xor    %edi,%edi
   73b79:	e8 e2 f4 ff ff       	callq  73060 <ggggc_collect0>
   73b7e:	f2 0f 10 05 7a b0 20 	movsd  0x20b07a(%rip),%xmm0        # 27ec00 <loadFactor>
   73b85:	00 
   73b86:	66 0f 2e 05 aa 35 00 	ucomisd 0x35aa(%rip),%xmm0        # 77138 <ggggc_null+0x1c8>
   73b8d:	00 
   73b8e:	77 32                	ja     73bc2 <ggggc_mallocRaw+0x2c2>
   73b90:	64 49 8b 04 24       	mov    %fs:(%r12),%rax
   73b95:	4c 8b 15 84 b0 20 00 	mov    0x20b084(%rip),%r10        # 27ec20 <currentPool>
   73b9c:	41 bf 01 00 00 00    	mov    $0x1,%r15d
   73ba2:	48 8b 00             	mov    (%rax),%rax
   73ba5:	64 49 89 04 24       	mov    %rax,%fs:(%r12)
   73baa:	e9 48 fe ff ff       	jmpq   739f7 <ggggc_mallocRaw+0xf7>
   73baf:	90                   	nop
   73bb0:	f2 0f 10 05 48 b0 20 	movsd  0x20b048(%rip),%xmm0        # 27ec00 <loadFactor>
   73bb7:	00 
   73bb8:	66 0f 2e 05 80 35 00 	ucomisd 0x3580(%rip),%xmm0        # 77140 <ggggc_null+0x1d0>
   73bbf:	00 
   73bc0:	76 0b                	jbe    73bcd <ggggc_mallocRaw+0x2cd>
   73bc2:	31 c0                	xor    %eax,%eax
   73bc4:	e8 37 f3 ff ff       	callq  72f00 <appendNewPool>
   73bc9:	85 c0                	test   %eax,%eax
   73bcb:	74 e3                	je     73bb0 <ggggc_mallocRaw+0x2b0>
   73bcd:	41 be 01 00 00 00    	mov    $0x1,%r14d
   73bd3:	eb bb                	jmp    73b90 <ggggc_mallocRaw+0x290>
   73bd5:	48 83 e7 fc          	and    $0xfffffffffffffffc,%rdi
   73bd9:	48 85 ff             	test   %rdi,%rdi
   73bdc:	49 89 7a 08          	mov    %rdi,0x8(%r10)
   73be0:	74 09                	je     73beb <ggggc_mallocRaw+0x2eb>
   73be2:	f6 07 02             	testb  $0x2,(%rdi)
   73be5:	0f 84 e2 00 00 00    	je     73ccd <ggggc_mallocRaw+0x3cd>
   73beb:	4d 89 c4             	mov    %r8,%r12
   73bee:	e9 39 fe ff ff       	jmpq   73a2c <ggggc_mallocRaw+0x12c>
   73bf3:	4b 8d 14 28          	lea    (%r8,%r13,1),%rdx
   73bf7:	48 29 d8             	sub    %rbx,%rax
   73bfa:	4d 89 c4             	mov    %r8,%r12
   73bfd:	48 89 3a             	mov    %rdi,(%rdx)
   73c00:	48 89 42 08          	mov    %rax,0x8(%rdx)
   73c04:	48 83 e2 fc          	and    $0xfffffffffffffffc,%rdx
   73c08:	49 89 52 08          	mov    %rdx,0x8(%r10)
   73c0c:	e9 1b fe ff ff       	jmpq   73a2c <ggggc_mallocRaw+0x12c>
   73c11:	31 c0                	xor    %eax,%eax
   73c13:	e8 e8 f2 ff ff       	callq  72f00 <appendNewPool>
   73c18:	85 c0                	test   %eax,%eax
   73c1a:	75 48                	jne    73c64 <ggggc_mallocRaw+0x364>
   73c1c:	4c 8b 15 05 b0 20 00 	mov    0x20b005(%rip),%r10        # 27ec28 <poolList>
   73c23:	e9 f8 fe ff ff       	jmpq   73b20 <ggggc_mallocRaw+0x220>
   73c28:	45 85 f6             	test   %r14d,%r14d
   73c2b:	74 1d                	je     73c4a <ggggc_mallocRaw+0x34a>
   73c2d:	45 31 e4             	xor    %r12d,%r12d
   73c30:	e9 56 fe ff ff       	jmpq   73a8b <ggggc_mallocRaw+0x18b>
   73c35:	0f 1f 00             	nopl   (%rax)
   73c38:	f2 0f 10 05 c0 af 20 	movsd  0x20afc0(%rip),%xmm0        # 27ec00 <loadFactor>
   73c3f:	00 
   73c40:	66 0f 2e 05 f8 34 00 	ucomisd 0x34f8(%rip),%xmm0        # 77140 <ggggc_null+0x1d0>
   73c47:	00 
   73c48:	76 0b                	jbe    73c55 <ggggc_mallocRaw+0x355>
   73c4a:	31 c0                	xor    %eax,%eax
   73c4c:	e8 af f2 ff ff       	callq  72f00 <appendNewPool>
   73c51:	85 c0                	test   %eax,%eax
   73c53:	74 e3                	je     73c38 <ggggc_mallocRaw+0x338>
   73c55:	4c 8b 15 c4 af 20 00 	mov    0x20afc4(%rip),%r10        # 27ec20 <currentPool>
   73c5c:	45 89 fe             	mov    %r15d,%r14d
   73c5f:	e9 93 fd ff ff       	jmpq   739f7 <ggggc_mallocRaw+0xf7>
   73c64:	48 8d 3d 89 34 00 00 	lea    0x3489(%rip),%rdi        # 770f4 <ggggc_null+0x184>
   73c6b:	45 31 e4             	xor    %r12d,%r12d
   73c6e:	e8 2d 05 f9 ff       	callq  41a0 <puts@plt>
   73c73:	e9 13 fe ff ff       	jmpq   73a8b <ggggc_mallocRaw+0x18b>
   73c78:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
   73c7f:	00 
   73c80:	48 8d 3d b1 33 00 00 	lea    0x33b1(%rip),%rdi        # 77038 <ggggc_null+0xc8>
   73c87:	45 31 e4             	xor    %r12d,%r12d
   73c8a:	e8 11 05 f9 ff       	callq  41a0 <puts@plt>
   73c8f:	e9 f7 fd ff ff       	jmpq   73a8b <ggggc_mallocRaw+0x18b>
   73c94:	49 8d 55 f8          	lea    -0x8(%r13),%rdx
   73c98:	49 8d 7c 24 08       	lea    0x8(%r12),%rdi
   73c9d:	49 89 42 10          	mov    %rax,0x10(%r10)
   73ca1:	31 f6                	xor    %esi,%esi
   73ca3:	49 c7 04 24 00 00 00 	movq   $0x0,(%r12)
   73caa:	00 
   73cab:	e8 50 05 f9 ff       	callq  4200 <memset@plt>
   73cb0:	e9 95 fd ff ff       	jmpq   73a4a <ggggc_mallocRaw+0x14a>
   73cb5:	48 8d 3d 55 34 00 00 	lea    0x3455(%rip),%rdi        # 77111 <ggggc_null+0x1a1>
   73cbc:	e8 df 04 f9 ff       	callq  41a0 <puts@plt>
   73cc1:	31 c0                	xor    %eax,%eax
   73cc3:	e8 d8 ef ff ff       	callq  72ca0 <assertParsableHeap>
   73cc8:	e8 93 04 f9 ff       	callq  4160 <abort@plt>
   73ccd:	48 8d 3d b4 33 00 00 	lea    0x33b4(%rip),%rdi        # 77088 <ggggc_null+0x118>
   73cd4:	e8 c7 04 f9 ff       	callq  41a0 <puts@plt>
   73cd9:	e8 82 04 f9 ff       	callq  4160 <abort@plt>
   73cde:	48 8d 3d 7b 33 00 00 	lea    0x337b(%rip),%rdi        # 77060 <ggggc_null+0xf0>
   73ce5:	e8 b6 04 f9 ff       	callq  41a0 <puts@plt>
   73cea:	e8 71 04 f9 ff       	callq  4160 <abort@plt>
   73cef:	e8 dc 04 f9 ff       	callq  41d0 <__stack_chk_fail@plt>
   73cf4:	66 90                	xchg   %ax,%ax
   73cf6:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
   73cfd:	00 00 00 