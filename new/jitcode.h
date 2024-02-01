#pragma once
#ifndef __JITFFI_JITCODE_H__
#define __JITFFI_JITCODE_H__

#include <stdint.h>
#include <stddef.h>

#if !defined(JITCODE_API)
#   define JITCODE_API(ident) jitcode_##ident
#endif

#endif
