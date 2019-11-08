#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "tokenizer.h"

enum State {
    STATE_ENABLED,
    STATE_DISABLED,
    STATE_INVALID,
};

Vector* preprocess(const Vector* vec);

#endif
