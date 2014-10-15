//
//  test-Signature.cc
//  snowcrash
//
//  Created by Pavan Kumar Sunkara on 10/15/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "snowcrashtest.h"
#include "Signature.h"

static const mdp::ByteBuffer PropertySignatureFixture = "id: 42 (yes, no) - a good message";
static const mdp::ByteBuffer EscapedPropertySignatureFixture = "`*id*(data):3`: `42` (yes, no) - a good message";

static const mdp::ByteBuffer ElementSignatureFixture = "42 (number) - a good number";
static const mdp::ByteBuffer EscapedElementSignatureFixture = "`*42*(data):3` (number) - a good number";

using namespace snowcrash;
using namespace scpl;

