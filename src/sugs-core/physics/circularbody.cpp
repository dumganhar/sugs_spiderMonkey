/* Copyright 2011 Jeffery Olson <olson.jeffery@gmail.com>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this list of
 *     conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list
 *     of conditions and the following disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JEFFERY OLSON <OLSON.JEFFERY@GMAIL.COM> ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * JEFFERY OLSON <OLSON.JEFFERY@GMAI`L.COM> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Jeffery Olson <olson.jeffery@gmail.com>.
 *
 */

#include "circularbody.h"

static JSBool
body_getPos(JSContext* cx, uintN argc, jsval* vp)
{
  JSObject* bodyObj = JS_THIS_OBJECT(cx, vp);

  cpBody* body = (cpBody*)JS_GetPrivate(cx, bodyObj);

  JSObject* posObj = JS_NewObject(cx, NULL, NULL, NULL);
  cpVect pos = cpBodyGetPos(body);
  jsval xPos = DOUBLE_TO_JSVAL(pos.x);
  jsval yPos = DOUBLE_TO_JSVAL(pos.y);
  if(!JS_SetProperty(cx, posObj, "x", &xPos)) {
     JS_ReportError(cx, "body_getPos: unable to set pos x val");
     return JS_FALSE;
  }
  if(!JS_SetProperty(cx, posObj, "y", &yPos)) {
     JS_ReportError(cx, "body_getPos: unable to set pos y val");
     return JS_FALSE;
  }

  jsval rVal = OBJECT_TO_JSVAL(posObj);
  JS_SET_RVAL(cx, vp, rVal);
  return JS_TRUE;
}

static void
getRadiusIterator(cpBody* body, cpShape* shape, void* data)
{
  *((cpFloat*)data) = cpCircleShapeGetRadius(shape);
}
static JSBool
circularBody_getRadius(JSContext* cx, uintN argc, jsval* vp)
{
  JSObject* bodyObj = JS_THIS_OBJECT(cx, vp);

  cpBody* body = (cpBody*)JS_GetPrivate(cx, bodyObj);

  cpFloat radius;
  // assuming only a single shape, here..
  cpBodyEachShape(body, &getRadiusIterator, &radius);
  jsval rVal = DOUBLE_TO_JSVAL(radius);
  JS_SET_RVAL(cx, vp, rVal);
  return JS_TRUE;
}

static JSBool
body_getRotation(JSContext* cx, uintN argc, jsval* vp)
{
  JSObject* bodyObj = JS_THIS_OBJECT(cx, vp);

  cpBody* body = (cpBody*)JS_GetPrivate(cx, bodyObj);

  jsval rVal = DOUBLE_TO_JSVAL(cpBodyGetAngle(body));
  JS_SET_RVAL(cx, vp, rVal);
  return JS_TRUE;
}
static JSBool
body_setRotation(JSContext* cx, uintN argc, jsval* vp)
{
  jsdouble angleRadiansVal;
  JSObject* spaceObj;
  if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "do", &angleRadiansVal, &spaceObj)) {
      /* Throw a JavaScript exception. */
      JS_ReportError(cx, "body_setRotation: couldn't parse out angle");
      return JS_FALSE;
  }

  JSObject* bodyObj = JS_THIS_OBJECT(cx, vp);

  cpBody* body = (cpBody*)JS_GetPrivate(cx, bodyObj);
  cpSpace* space = (cpSpace*)JS_GetPrivate(cx, spaceObj);
  cpBodySetAngle(body, angleRadiansVal);
  cpSpaceReindexShapesForBody(space, body);

  jsval rVal = JSVAL_VOID;
  JS_SET_RVAL(cx, vp, rVal);
  return JS_TRUE;
}

static JSBool
body_applyDirectionalImpulse(JSContext* cx, uintN argc, jsval* vp)
{
  jsdouble amt;
  if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &amt)) {
      /* Throw a JavaScript exception. */
      JS_ReportError(cx, "body_applyDirectionalImpulse: couldn't parse out amt");
      return JS_FALSE;
  }

  JSObject* bodyObj = JS_THIS_OBJECT(cx, vp);

  cpBody* body = (cpBody*)JS_GetPrivate(cx, bodyObj);
  cpFloat angle = cpBodyGetAngle(body);
  double xAmt = sin(angle) * amt;
  double yAmt = cos(angle) * amt;
  cpVect j = {xAmt, yAmt};
  cpBodyApplyImpulse(body, j, cpvzero);

  jsval rVal = JSVAL_VOID;
  JS_SET_RVAL(cx, vp, rVal);
  return JS_TRUE;
}

static JSFunctionSpec
circularBody_functionSpec[] = {
  JS_FS("__native_getPos", body_getPos, 0, 0),
  JS_FS("__native_getRadius", circularBody_getRadius, 0, 0),
  JS_FS("__native_getRotation", body_getRotation, 0, 0),
  JS_FS("__native_setRotation", body_setRotation, 1, 0),
  JS_FS("__native_applyDirectionalImpulse", body_applyDirectionalImpulse, 1, 0),
  JS_FS_END
};

static void
removeShapeFromBodyIterator(cpBody* body, cpShape* shape, void* data)
{
  printf("removing shape...\n");
  cpShapeFree(shape);
}
static void
removeConstraintFromBodyIterator(cpBody* body, cpConstraint* constraint, void* data)
{
  cpConstraintFree(constraint);
}

void
classdef_circularBody_finalize(JSContext* cx, JSObject* sp) {
  cpBody* body = (cpBody*)JS_GetPrivate(cx, sp);
  printf("About to try and delete a cpBody and, its cpConstraints and its cpShapes...\n");
  cpBodyEachShape(body, &removeShapeFromBodyIterator, 0);
  cpBodyEachConstraint(body, &removeConstraintFromBodyIterator, 0);
  cpBodyFree(body);
}

static JSClass
circularBodyClassDef = {
  "NativeChipmunkCircularBody",
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
  JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, classdef_circularBody_finalize
};

JSObject*
sugs::physics::createNewCircularBodyFrom(JSContext* cx, cpSpace* space, cpFloat xPos, cpFloat yPos, cpFloat mass, cpFloat radius, cpFloat friction, unsigned int groupId, unsigned int collisionType, JSObject* outterJsObj)
{
  cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
  cpBody* circularBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
  cpBodySetPos(circularBody, cpv(xPos, yPos));
  cpShape* circularShape = cpSpaceAddShape(space, cpCircleShapeNew(circularBody, radius, cpvzero));
  cpShapeSetFriction(circularShape, friction);
  cpShapeSetGroup(circularShape, groupId);
  cpShapeSetCollisionType(circularShape, collisionType);

  cpBodySetUserData(circularBody, outterJsObj);

  JSObject* bodyObj = JS_NewObject(cx, &circularBodyClassDef, NULL, NULL);
  JS_DefineFunctions(cx, bodyObj, circularBody_functionSpec);

  if(!JS_SetPrivate(cx, bodyObj, (void*)circularBody)) {
    JS_ReportError(cx, "createNewCircularBodyFrom: Failed to set private for new circular body..");
    return NULL;
  }

  return bodyObj;
}