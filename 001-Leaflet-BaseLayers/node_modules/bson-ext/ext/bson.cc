//===========================================================================

#include <stdarg.h>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <stdlib.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include <v8.h>

// this and the above block must be around the v8.h header otherwise
// v8 is not happy
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <node.h>
#include <node_version.h>
#include <node_buffer.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <errno.h>

#if defined(__sun) || defined(_AIX)
	#include <alloca.h>
#endif

#include "bson.h"

void die(const char *message) {
	if(errno) {
		perror(message);
	} else {
		printf("ERROR: %s\n", message);
	}

	exit(1);
}

//===========================================================================

// Equality Objects
static const char* LONG_CLASS_NAME = "Long";
static const char* OBJECT_ID_CLASS_NAME = "ObjectID";
static const char* BINARY_CLASS_NAME = "Binary";
static const char* CODE_CLASS_NAME = "Code";
static const char* DBREF_CLASS_NAME = "DBRef";
static const char* SYMBOL_CLASS_NAME = "Symbol";
static const char* DOUBLE_CLASS_NAME = "Double";
static const char* TIMESTAMP_CLASS_NAME = "Timestamp";
static const char* MIN_KEY_CLASS_NAME = "MinKey";
static const char* MAX_KEY_CLASS_NAME = "MaxKey";
static const char* REGEXP_CLASS_NAME = "BSONRegExp";
static const char* DECIMAL128_CLASS_NAME = "Decimal128";
static const char* INT32_CLASS_NAME = "Int32";

// Equality speed up comparison objects
static const char* BSONTYPE_PROPERTY_NAME = "_bsontype";
static const char* LONG_LOW_PROPERTY_NAME = "low_";
static const char* LONG_HIGH_PROPERTY_NAME = "high_";
static const char* OBJECT_ID_ID_PROPERTY_NAME = "id";
static const char* BINARY_POSITION_PROPERTY_NAME = "position";
static const char* BINARY_SUBTYPE_PROPERTY_NAME = "sub_type";
static const char* BINARY_BUFFER_PROPERTY_NAME = "buffer";
static const char* SYMBOL_VALUE_PROPERTY_NAME = "value";
static const char* DECIMAL128_VALUE_PROPERTY_NAME = "bytes";
static const char* DOUBLE_VALUE_PROPERTY_NAME = "value";
static const char* INT32_VALUE_PROPERTY_NAME = "value";

static const char* DBREF_REF_PROPERTY_NAME = "$ref";
static const char* DBREF_ID_REF_PROPERTY_NAME = "$id";
static const char* DBREF_DB_REF_PROPERTY_NAME = "$db";
static const char* DBREF_NAMESPACE_PROPERTY_NAME = "namespace";
static const char* DBREF_DB_PROPERTY_NAME = "db";
static const char* DBREF_OID_PROPERTY_NAME = "oid";
static const char* REGEX_PATTERN_PROPERTY_NAME = "pattern";
static const char* REGEX_OPTIONS_PROPERTY_NAME = "options";

static const char* CODE_CODE_PROPERTY_NAME = "code";
static const char* CODE_SCOPE_PROPERTY_NAME = "scope";
static const char* TO_BSON_PROPERTY_NAME = "toBSON";
static const char* TO_OBJECT_PROPERTY_NAME = "toObject";

void DataStream::WriteObjectId(const Local<Object>& object, const Local<String>& key)
{
	uint16_t buffer[12];
	Nan::MaybeLocal<v8::Value> obj = NanGet(object, key);

	if(node::Buffer::HasInstance(obj.ToLocalChecked())) {
		uint32_t length = (uint32_t)node::Buffer::Length(obj.ToLocalChecked());
		this->WriteData(node::Buffer::Data(obj.ToLocalChecked()), length);
	} else {
		NanGet(object, key)->ToString()->Write(buffer, 0, 12);

		for(uint32_t i = 0; i < 12; ++i)
		{
			*p++ = (char) buffer[i];
		}
	}
}

void ThrowAllocatedStringException(size_t allocationSize, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char* string = (char*) malloc(allocationSize);
	if(string == NULL) die("Failed to allocate ThrowAllocatedStringException");
	vsprintf(string, format, args);
	va_end(args);
	throw string;
}

void DataStream::CheckKey(const Local<String>& keyName)
{
	size_t keyLength = keyName->Utf8Length();
	if(keyLength == 0) return;

	// Allocate space for the key, do not need to zero terminate as WriteUtf8 does it
	char* keyStringBuffer = (char*) alloca(keyLength + 1);
	// Write the key to the allocated buffer
	keyName->WriteUtf8(keyStringBuffer);
	// Check for the zero terminator
	char* terminator = strchr(keyStringBuffer, 0x00);

	// If the location is not at the end of the string we've got an illegal 0x00 byte somewhere
	if(terminator != &keyStringBuffer[keyLength]) {
		ThrowAllocatedStringException(64+keyLength, "key %s must not contain null bytes", keyStringBuffer);
	}

	if(keyStringBuffer[0] == '$')
	{
		ThrowAllocatedStringException(64+keyLength, "key %s must not start with '$'", keyStringBuffer);
	}

	if(strchr(keyStringBuffer, '.') != NULL)
	{
		ThrowAllocatedStringException(64+keyLength, "key %s must not contain '.'", keyStringBuffer);
	}
}

void DataStream::CheckForIllegalString(const Local<String>& keyName)
{
	size_t keyLength = keyName->Utf8Length();
	if(keyLength == 0) return;

	// Allocate space for the key, do not need to zero terminate as WriteUtf8 does it
	char* keyStringBuffer = (char*) alloca(keyLength + 1);
	// Write the key to the allocated buffer
	keyName->WriteUtf8(keyStringBuffer);
	// Check for the zero terminator
	char* terminator = strchr(keyStringBuffer, 0x00);

	// If the location is not at the end of the string we've got an illegal 0x00 byte somewhere
	if(terminator != &keyStringBuffer[keyLength]) {
		ThrowAllocatedStringException(64+keyLength, "key %s must not contain null bytes", keyStringBuffer);
	}
}

template<typename T> void BSONSerializer<T>::SerializeDocument(const Local<Value>& value)
{
	bool valid = this->BeginDocument();
	if(!valid) {
		return ThrowAllocatedStringException(64, "cyclic dependency detected");
	}

	void* documentSize = this->BeginWriteSize();
	Local<Object> object = bson->GetSerializeObject(value);
	Local<String> propertyName;
	Local<Value> propertyValue;

	if(!NanHas(object, "entries")) {
		// Get the object property names
	  Local<Array> propertyNames = object->GetPropertyNames();

		// Length of the property
		int propertyLength = propertyNames->Length();
		for(int i = 0;  i < propertyLength; ++i)
		{
			propertyName = NanGet(propertyNames, i)->ToString();
			if(checkKeys) this->CheckKey(propertyName);
			propertyValue = NanGet(object, propertyName);

			// We are not serializing the function
			if(!serializeFunctions && propertyValue->IsFunction()) {
				continue;
			}

			// We are ignoring undefined values
			if(ignoreUndefined && propertyValue->IsUndefined()) {
				continue;
			}

			// Serialize the value
			void* typeLocation = this->BeginWriteType();
			this->WriteString(propertyName);
			SerializeValue(typeLocation, propertyValue, false);
		}
	} else {
		// Get the entries function
		const Local<Value>& entries = NanGet(object, "entries");
		if(!entries->IsFunction()) ThrowAllocatedStringException(64, "Map.entries is not a function");

		// Get the iterator
		Local<Object> iterator = Local<Function>::Cast(entries)->Call(object, 0, NULL)->ToObject();

		// Check if we have the next value
		const Local<Value>& next = NanGet(iterator, "next");
		if(!next->IsFunction()) ThrowAllocatedStringException(64, "Map.iterator.next is not a function");

		// Iterate until we are done
		while(true) {
			// Get the entry
			Local<Object> entry = Local<Function>::Cast(next)->Call(iterator, 0, NULL)->ToObject();

			// Check if we are done
			if(NanGet(entry, "done")->ToBoolean()->Value()) {
				break;
			}

			// Get the value object
			Local<Object> value = NanGet(entry, "value")->ToObject();

			// Not done then lets get the value items
			propertyName = NanGet(value, "0")->ToString();
			if(checkKeys) this->CheckKey(propertyName);
			// Get the property value
			propertyValue = NanGet(value, "1");

			// We are not serializing the function
			if(!serializeFunctions && propertyValue->IsFunction()) {
				continue;
			}

			// We are ignoring undefined values
			if(ignoreUndefined && propertyValue->IsUndefined()) {
				continue;
			}

			// Serialize the value
			void* typeLocation = this->BeginWriteType();
			this->WriteString(propertyName);
			SerializeValue(typeLocation, propertyValue, false);
		}
	}

	this->WriteByte(0);
	this->CommitSize(documentSize);
	this->EndDocument();
}

template<typename T> void BSONSerializer<T>::SerializeArray(const Local<Value>& value)
{
	void* documentSize = this->BeginWriteSize();

	Local<Array> array = Local<Array>::Cast(value->ToObject());
	uint32_t arrayLength = array->Length();

	for(uint32_t i = 0;  i < arrayLength; ++i)
	{
		void* typeLocation = this->BeginWriteType();
		this->WriteUInt32String(i);

		if(i >= preLoadedIndex) {
			SerializeValue(typeLocation, NanGet(array, i), true);
		} else {
			SerializeValue(typeLocation, NanGet(array, BSON::indexesStrings[i]), true);
		}
	}

	this->WriteByte(0);
	this->CommitSize(documentSize);
}

// This is templated so that we can use this function to both count the number of bytes, and to serialize those bytes.
// The template approach eliminates almost all of the inspection of values unless they're required (eg. string lengths)
// and ensures that there is always consistency between bytes counted and bytes written by design.
template<typename T> void BSONSerializer<T>::SerializeValue(void* typeLocation, const Local<Value> constValue, bool isArray)
{
	Local<Value> value = constValue;

	// Process all the values
	if(value->IsNumber())
	{
		double doubleValue = value->NumberValue();
		int intValue = (int) doubleValue;
		if(intValue == doubleValue)
		{
			this->CommitType(typeLocation, BSON_TYPE_INT);
			this->WriteInt32(intValue);
		}
		else
		{
			this->CommitType(typeLocation, BSON_TYPE_NUMBER);
			this->WriteDouble(doubleValue);
		}
	}
	else if(value->IsString())
	{
		this->CommitType(typeLocation, BSON_TYPE_STRING);
		this->WriteLengthPrefixedString(value->ToString());
	}
	else if(value->IsDate())
	{
		this->CommitType(typeLocation, BSON_TYPE_DATE);
		this->WriteInt64(value->NumberValue());
	}
	else if(value->IsBoolean())
	{
		this->CommitType(typeLocation, BSON_TYPE_BOOLEAN);
		this->WriteBool(value);
	}
	else if(value->IsArray())
	{
		this->CommitType(typeLocation, BSON_TYPE_ARRAY);
		SerializeArray(value);
	}
	else if(value->IsObject())
	{
		Local<Object> object = value->ToObject();

		if(NanHas(object, BSONTYPE_PROPERTY_NAME))
		{
			const Local<String>& constructorString = NanGet(object, BSONTYPE_PROPERTY_NAME)->ToString();

			if(Nan::New(bson->LONG_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_LONG);
				this->WriteInt32(object, Nan::New(bson->LONG_LOW_PROPERTY_NAME_STR));
				this->WriteInt32(object, Nan::New(bson->LONG_HIGH_PROPERTY_NAME_STR));
			}
			else if(Nan::New(bson->TIMESTAMP_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_TIMESTAMP);
				this->WriteInt32(object, Nan::New(bson->LONG_LOW_PROPERTY_NAME_STR));
				this->WriteInt32(object, Nan::New(bson->LONG_HIGH_PROPERTY_NAME_STR));
			}
			else if(Nan::New(bson->OBJECT_ID_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_OID);
				this->WriteObjectId(object, Nan::New(bson->OBJECT_ID_ID_PROPERTY_NAME_STR));
			}
			else if(Nan::New(bson->BINARY_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_BINARY);

				uint32_t length = NanGet(object, Nan::New(bson->BINARY_POSITION_PROPERTY_NAME_STR))->Uint32Value();
				Local<Object> bufferObj = NanGet(object, Nan::New(bson->BINARY_BUFFER_PROPERTY_NAME_STR))->ToObject();

				// Add the deprecated 02 type 4 bytes of size to total
				if(NanGet(object, Nan::New(bson->BINARY_SUBTYPE_PROPERTY_NAME_STR))->Int32Value() == 0x02) {
					length = length + 4;
				}

				this->WriteInt32(length);
				this->WriteByte(object, Nan::New(bson->BINARY_SUBTYPE_PROPERTY_NAME_STR));	// write subtype

				// If type 0x02 write the array length aswell
				if(NanGet(object, Nan::New(bson->BINARY_SUBTYPE_PROPERTY_NAME_STR))->Int32Value() == 0x02) {
					length = length - 4;
					this->WriteInt32(length);
				}

				// Write the actual data
				this->WriteData(node::Buffer::Data(bufferObj), length);
			}
			else if(Nan::New(bson->DECIMAL128_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_DECIMAL128);
				// Get the bytes length
				uint32_t length = (uint32_t)node::Buffer::Length(NanGet(object, Nan::New(bson->DECIMAL128_VALUE_PROPERTY_NAME_STR)));
				// Get the bytes buffer object
				Local<Object> bufferObj = NanGet(object, Nan::New(bson->DECIMAL128_VALUE_PROPERTY_NAME_STR))->ToObject();
				// Write the actual decimal128 bytes
				this->WriteData(node::Buffer::Data(bufferObj), length);
			}
			else if(Nan::New(bson->DOUBLE_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_NUMBER);
				this->WriteDouble(object, Nan::New(bson->DOUBLE_VALUE_PROPERTY_NAME_STR));
			}
			else if(Nan::New(bson->INT32_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_INT);
				this->WriteInt32(object, Nan::New(bson->INT32_VALUE_PROPERTY_NAME_STR));
			}
			else if(Nan::New(bson->SYMBOL_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_SYMBOL);
				this->WriteLengthPrefixedString(NanGet(object, Nan::New(bson->SYMBOL_VALUE_PROPERTY_NAME_STR))->ToString());
			}
			else if(Nan::New(bson->CODE_CLASS_NAME_STR)->Equals(constructorString))
			{
				const Local<String>& function = NanGet(object, Nan::New(bson->CODE_CODE_PROPERTY_NAME_STR))->ToString();
				// Does the code object have a defined scope
				bool hasScope = NanHas(object, Nan::New(bson->CODE_SCOPE_PROPERTY_NAME_STR))
					&& NanGet(object, Nan::New(bson->CODE_SCOPE_PROPERTY_NAME_STR))->IsObject()
					&& !NanGet(object, Nan::New(bson->CODE_SCOPE_PROPERTY_NAME_STR))->IsNull();

				if(hasScope)
				{
					const Local<Object>& scope = NanGet(object, Nan::New(bson->CODE_SCOPE_PROPERTY_NAME_STR))->ToObject();
					this->CommitType(typeLocation, BSON_TYPE_CODE_W_SCOPE);
					void* codeWidthScopeSize = this->BeginWriteSize();
					this->WriteLengthPrefixedString(function->ToString());
					SerializeDocument(scope);
					this->CommitSize(codeWidthScopeSize);
				}
				else
				{
					this->CommitType(typeLocation, BSON_TYPE_CODE);
					this->WriteLengthPrefixedString(function->ToString());
				}
			}
			else if(Nan::New(bson->DBREF_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_OBJECT);

				void* dbRefSize = this->BeginWriteSize();

				void* refType = this->BeginWriteType();
				this->WriteData("$ref", 5);
				SerializeValue(refType, NanGet(object, Nan::New(bson->DBREF_NAMESPACE_PROPERTY_NAME_STR)), false);

				void* idType = this->BeginWriteType();
				this->WriteData("$id", 4);
				SerializeValue(idType, NanGet(object, Nan::New(bson->DBREF_OID_PROPERTY_NAME_STR)), false);

				const Local<Value>& refDbValue = NanGet(object, Nan::New(bson->DBREF_DB_PROPERTY_NAME_STR));
				if(!refDbValue->IsUndefined())
				{
					void* dbType = this->BeginWriteType();
					this->WriteData("$db", 4);
					SerializeValue(dbType, refDbValue, false);
				}

				this->WriteByte(0);
				this->CommitSize(dbRefSize);
			}
			else if(Nan::New(bson->REGEXP_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_REGEXP);
				// Get the pattern string
				Local<String> pattern = NanGet(object, Nan::New(bson->REGEX_PATTERN_PROPERTY_NAME_STR))->ToString();
				// Validate if the pattern is valid
				this->CheckForIllegalString(pattern);
				// Write the 0 terminated string
				this->WriteString(pattern);
				// Get the options string
				Local<String> options = NanGet(object, Nan::New(bson->REGEX_OPTIONS_PROPERTY_NAME_STR))->ToString();
				// Validate if the options is valid
				this->CheckForIllegalString(options);
				// Write the 0 terminated string
				this->WriteString(options);
			}
			else if(Nan::New(bson->MIN_KEY_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_MIN_KEY);
			}
			else if(Nan::New(bson->MAX_KEY_CLASS_NAME_STR)->Equals(constructorString))
			{
				this->CommitType(typeLocation, BSON_TYPE_MAX_KEY);
			}
		}
		else if(value->IsFunction())
		{
			this->CommitType(typeLocation, BSON_TYPE_CODE);
			this->WriteLengthPrefixedString(value->ToString());
		}
		else if(node::Buffer::HasInstance(value))
		{
			this->CommitType(typeLocation, BSON_TYPE_BINARY);

	    #if NODE_MAJOR_VERSION == 0 && NODE_MINOR_VERSION < 3
       Local<Object> buffer = ObjectWrap::Unwrap<Buffer>(value->ToObject());
			 uint32_t length = object->length();
	    #else
			 uint32_t length = (uint32_t)node::Buffer::Length(value->ToObject());
	    #endif

			this->WriteInt32(length);
			this->WriteByte(0);
			this->WriteData(node::Buffer::Data(value->ToObject()), length);
		}
		else if(value->IsRegExp())
		{
			this->CommitType(typeLocation, BSON_TYPE_REGEXP);
			const Local<RegExp>& regExp = Local<RegExp>::Cast(value);
			const Local<String> regExpString = regExp->GetSource()->ToString();

			// Validate if the string is valid
			this->CheckForIllegalString(regExpString);

			// Write the regular expression string
			this->WriteString(regExpString);

			// Unpack the flags
			int flags = regExp->GetFlags();
			if(flags & RegExp::kGlobal) this->WriteByte('s');
			if(flags & RegExp::kIgnoreCase) this->WriteByte('i');
			if(flags & RegExp::kMultiline) this->WriteByte('m');
			return this->WriteByte(0);
		}
		else
		{
			// Check for toBSON function
			if(NanHas(object, "toBSON")) {
				const Local<Value>& toBSON = NanGet(object, "toBSON");
				if(!toBSON->IsFunction()) ThrowAllocatedStringException(64, "toBSON is not a function");
				value = Local<Function>::Cast(toBSON)->Call(object, 0, NULL);
				return SerializeValue(typeLocation, value, false);
			}

			this->CommitType(typeLocation, BSON_TYPE_OBJECT);
			SerializeDocument(value);
		}
	}
	else if(value->IsNull())
	{
		this->CommitType(typeLocation, BSON_TYPE_NULL);
	}
	else if(value->IsUndefined() && !isArray && !ignoreUndefined)
	{
		this->CommitType(typeLocation, BSON_TYPE_NULL);
	}
	else if(value->IsUndefined() && isArray && ignoreUndefined)
	{
		this->CommitType(typeLocation, BSON_TYPE_NULL);
	}
	else if(value->IsUndefined() && isArray && !ignoreUndefined)
	{
		this->CommitType(typeLocation, BSON_TYPE_NULL);
	}
}

// Data points to start of element list, length is length of entire document including '\0' but excluding initial size
BSONDeserializer::BSONDeserializer(BSON* aBson, char* data, size_t length, bool bsonRegExp, bool promoteLongs, bool promoteBuffers, bool promoteValues, Local<Object> fieldsAsRaw)
: bson(aBson),
  pStart(data),
  p(data),
  pEnd(data + length - 1),
	bsonRegExp(bsonRegExp),
	promoteLongs(promoteLongs),
	promoteBuffers(promoteBuffers),
	promoteValues(promoteValues),
	fieldsAsRaw(fieldsAsRaw)
{
	if(*pEnd != '\0') ThrowAllocatedStringException(64, "Missing end of document marker '\\0'");
}

BSONDeserializer::BSONDeserializer(BSONDeserializer& parentSerializer, size_t length, bool bsonRegExp, bool promoteLongs, bool promoteBuffers, bool promoteValues, Local<Object> fieldsAsRaw)
: bson(parentSerializer.bson),
  pStart(parentSerializer.p),
  p(parentSerializer.p),
  pEnd(parentSerializer.p + length - 1),
	bsonRegExp(bsonRegExp),
	promoteLongs(promoteLongs),
	promoteBuffers(promoteBuffers),
	promoteValues(promoteValues),
	fieldsAsRaw(fieldsAsRaw)
{
	parentSerializer.p += length;
	if(pEnd > parentSerializer.pEnd) ThrowAllocatedStringException(64, "Child document exceeds parent's bounds");
	if(*pEnd != '\0') ThrowAllocatedStringException(64, "Missing end of document marker '\\0'");
}

Local<Value> BSONDeserializer::ReadCString() {
	char* start = p;
	while(*p++ && (p < pEnd)) { }
	if(p > pEnd) {
		ThrowAllocatedStringException(64, "Illegal CString found");
	}

	return Unmaybe(Nan::New<String>(start, (int32_t) (p-start-1) ));
}

int32_t BSONDeserializer::ReadJavascriptRegexOptions() {
	int32_t options = 0;
	for(;;) {
		switch(*p++) {
			case '\0': return options;
			case 's': options |= RegExp::kGlobal; break;
			case 'i': options |= RegExp::kIgnoreCase; break;
			case 'm': options |= RegExp::kMultiline; break;
			default:
				ThrowAllocatedStringException(64, "Javascript RegExp option not one of s, i, m");
				break;
		}
	}
}

int32_t BSONDeserializer::ReadBSONRegexOptions() {
	int32_t options = 0;
	for(;;) {
		switch(*p++) {
			case '\0': return options;
			case 's': break;
			case 'i': break;
			case 'l': break;
			case 'u': break;
			case 'x': break;
			case 'm': break;
			default:
				ThrowAllocatedStringException(64, "BSON RegExp option not one of s, i, l, u, x, m");
				break;
		}
	}
}

void BSONDeserializer::ReadIntegerString() {
	while(*p) {
		p++;
	}
	++p;
}

Local<String> BSONDeserializer::ReadString() {
	int32_t length = ReadInt32();

	if(length <= 0 || length > (pEnd - p)) {
		ThrowAllocatedStringException(64, "Invalid bson string length");
	}

	char* start = p;
	p += length;
	if(*(p - 1) != 0x00) {
		ThrowAllocatedStringException(64, "Illegal bson string terminator found");
	}

	return Unmaybe(Nan::New<String>(start, length-1));
}

Local<Object> BSONDeserializer::ReadObjectId() {
	// Copy the data into a buffer
	Local<Object> buffer = Unmaybe(Nan::CopyBuffer(p, 12));
	// Move pointer
	p += 12;
	// Return the buffer
	return Unmaybe(buffer);
}

Local<Value> BSONDeserializer::DeserializeDocument(bool raw) {
	const char* start = p;

	uint32_t length = ReadUInt32();

	if(length < 5) ThrowAllocatedStringException(64, "Bad BSON: Document is less than 5 bytes");

	// Do we deserialize as a raw value
	if(raw) {
		// Copy the whole bon object
		Local<Object> buffer = Unmaybe(Nan::CopyBuffer(p - 4, length));
		// Adjust the length to the end of the object
		p += length - 4;
		// Return the new buffer
		return buffer;
	}

	BSONDeserializer documentDeserializer(*this, length-4, bsonRegExp, promoteLongs, promoteBuffers, promoteValues, fieldsAsRaw);
	// Serialize the document
	Local<Value> value = documentDeserializer.DeserializeDocumentInternal();

	if(length != (p - start)) {
		ThrowAllocatedStringException(64, "Illegal Document Length");
	}

	// Return the value
	return value;
}

Local<Value> BSONDeserializer::DeserializeDocumentInternal() {
	Local<Object> returnObject = Unmaybe(Nan::New<Object>());
	Local<String> propertyName;
	bool raw = false;

	while(HasMoreData()) {
		BsonType type = (BsonType) ReadByte();
		const Local<Value>& name = ReadCString();
		if(name->IsNull()) ThrowAllocatedStringException(64, "Bad BSON Document: illegal CString");

		// Special case for fieldsAsRaw matching the name and the value being a ARRAY
		if(type == BSON_TYPE_ARRAY && !Nan::MaybeLocal<Object>(fieldsAsRaw).IsEmpty()) {
			// Get the object property names
		  Local<Array> propertyNames = fieldsAsRaw->GetPropertyNames();

			// Length of the property
			int propertyLength = propertyNames->Length();
			for(int i = 0;  i < propertyLength; ++i)
			{
				propertyName = NanGet(propertyNames, i)->ToString();

				// We found a field that matches name wise, we now want to return a raw buffer
				// instead of deserializing the array;
				if(name->StrictEquals(propertyName)) {
					raw = true;
				}
			}
		}

		// Deserialize the value
		const Local<Value>& value = DeserializeValue(type, raw);
		returnObject->Set(name, value);
	}

	if(p != pEnd) ThrowAllocatedStringException(64, "Bad BSON Document: Serialize consumed unexpected number of bytes");

	// From JavaScript:
	// if(object['$id'] != null) object = new DBRef(object['$ref'], object['$id'], object['$db']);
	if(NanHas(returnObject, DBREF_ID_REF_PROPERTY_NAME)) {
		Local<Value> argv[] = { NanGet(returnObject, DBREF_REF_PROPERTY_NAME), NanGet(returnObject, DBREF_ID_REF_PROPERTY_NAME), NanGet(returnObject, DBREF_DB_REF_PROPERTY_NAME) };
		Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->dbrefConstructor), 3, argv);
		return obj.ToLocalChecked();
	} else {
		return returnObject;
	}
}

Local<Value> BSONDeserializer::DeserializeArray(bool raw) {
	uint32_t length = ReadUInt32();
	if(length < 5) ThrowAllocatedStringException(64, "Bad BSON: Array Document is less than 5 bytes");

	BSONDeserializer documentDeserializer(*this, length-4, bsonRegExp, promoteLongs, promoteBuffers, promoteValues, fieldsAsRaw);
	return documentDeserializer.DeserializeArrayInternal(raw);
}

Local<Value> BSONDeserializer::DeserializeArrayInternal(bool raw) {
	Local<Array> returnArray = Unmaybe(Nan::New<Array>());
	uint32_t index = 0;

	while(HasMoreData()) {
		BsonType type = (BsonType) ReadByte();
		ReadIntegerString();
		const Local<Value>& value = DeserializeValue(type, raw);

		if(index >= preLoadedIndex) {
			returnArray->Set(Nan::New<Integer>(index++), value);
		} else {
			returnArray->Set(Nan::New(BSON::indexes[index++]), value);
		}
	}

	if(p != pEnd) ThrowAllocatedStringException(64, "Bad BSON Array: Serialize consumed unexpected number of bytes");
	return returnArray;
}

Local<Value> BSONDeserializer::DeserializeValue(BsonType type, bool raw)
{
	switch(type)
	{
	case BSON_TYPE_STRING: {
		return ReadString();
	}

	case BSON_TYPE_INT: {
		if((pEnd - p) < 4) {
			ThrowAllocatedStringException(64, "Illegal BSON found, truncated Int32");
		}

		Local<Value> value = Nan::New<Integer>(ReadInt32());

		if (!promoteValues) {
			Local<Value> argv[] = { value };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->int32Constructor), 1, argv);
			return obj.ToLocalChecked();
		}

		return value;
	}

	case BSON_TYPE_NUMBER: {
		if((pEnd - p) < 8) {
			ThrowAllocatedStringException(64, "Illegal BSON found, truncated Double");
		}

		Local<Value> value = Nan::New<Number>(ReadDouble());

		if (!promoteValues) {
			Local<Value> argv[] = { value };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->doubleConstructor), 1, argv);
			return obj.ToLocalChecked();
		}

		return value;
	}

	case BSON_TYPE_NULL:
		return Nan::Null();

	case BSON_TYPE_UNDEFINED:
		return Nan::Undefined();

	case BSON_TYPE_TIMESTAMP: {
			if((pEnd - p) < 8) {
				ThrowAllocatedStringException(64, "Illegal BSON found, truncated Timestamp");
			}

			int32_t lowBits = ReadInt32();
			int32_t highBits = ReadInt32();
			Local<Value> argv[] = { Nan::New<Int32>(lowBits), Nan::New<Int32>(highBits) };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->timestampConstructor), 2, argv);
			return obj.ToLocalChecked();
		}

	case BSON_TYPE_BOOLEAN: {
		char value = ReadByte();

		if(value != 0 && value != 1) {
			ThrowAllocatedStringException(64, "Illegal BSON Binary value");
		}

		return (value != 0) ? Nan::True() : Nan::False();
	}

	case BSON_TYPE_REGEXP: {
			const Local<Value>& regex = ReadCString();
			if(regex->IsNull()) ThrowAllocatedStringException(64, "Bad BSON Document: illegal CString");

			if (bsonRegExp) {
				// Save pointer
				char* start = p;
				// Read and validate options
				ReadBSONRegexOptions();
				// Reset read pointer
				p = start;
				// Read the options
				const Local<Value>& options = ReadCString();
				Local<Value> argv[] = { regex, options };
				Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->regexpConstructor), 2, argv);
				return obj.ToLocalChecked();
			} else {
				int32_t options = ReadJavascriptRegexOptions();
				return Unmaybe(Nan::New<RegExp>(regex->ToString(), (RegExp::Flags) options));
			}
		}

	case BSON_TYPE_CODE: {
			const Local<String>& code = ReadString();
			Local<Value> argv[] = { code, Nan::Undefined() };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->codeConstructor), 2, argv);
			return obj.ToLocalChecked();
		}

	case BSON_TYPE_CODE_W_SCOPE: {
			char* const start = p;
			int32_t length = ReadInt32();

			if(length < (4+4+4+1)) {
				ThrowAllocatedStringException(64, "code_w_scope total size shorter minimum expected length");
			}

			const Local<String>& code = ReadString();
			const Local<Value>& scope = DeserializeDocument(false);
			char* const end = p;

			if((end-start) != length) {
				ThrowAllocatedStringException(64, "code_w_scope total size shorter minimum expected length");
			}

			Local<Value> argv[] = { code, scope->ToObject() };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->codeConstructor), 2, argv);
			return obj.ToLocalChecked();
		}

	case BSON_TYPE_OID: {
			if((pEnd - p) < 12) {
				ThrowAllocatedStringException(64, "Illegal BSON found, truncated ObjectId");
			}

			Local<Value> argv[] = { ReadObjectId() };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->objectIDConstructor), 1, argv);
			return obj.ToLocalChecked();
		}

	case BSON_TYPE_BINARY: {
			int32_t length = ReadInt32();

			if(length < 0) {
				ThrowAllocatedStringException(64, "binary bson object length must be >= 0");
			}

			// Illegal binary size
			if(length > ((pEnd - p) + 4)) {
				ThrowAllocatedStringException(64, "binary bson object size is longer than bson message");
			}

			uint32_t subType = ReadByte();
			if(subType == 0x02) {
				length = ReadInt32();

				if(length < 0) {
					ThrowAllocatedStringException(64, "binary bson type 0x02 object length must be >= 0");
				}

				// Illegal binary size
				if(length > (pEnd - p)) {
					ThrowAllocatedStringException(64, "binary bson object type 0x02 size is longer than bson message");
				}
			}

			Local<Object> buffer = Unmaybe(Nan::CopyBuffer(p, length));
			p += length;

			if (promoteBuffers && promoteValues) {
				return buffer;
			} else {
				Local<Value> argv[] = { buffer, Nan::New<Uint32>(subType) };
				Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->binaryConstructor), 2, argv);
				return obj.ToLocalChecked();
			}
		}

	case BSON_TYPE_DECIMAL128: {
		if((pEnd - p) < 16) {
			ThrowAllocatedStringException(64, "Illegal BSON found, truncated Decimal128");
		}

		// Read the 16 bytes making up the decimal 128 value
		Local<Object> buffer = Unmaybe(Nan::CopyBuffer(p, 16));
		p += 16;
		Local<Value> argv[] = { buffer };
		Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->decimalConstructor), 1, argv);
		Local<Object> object = obj.ToLocalChecked()->ToObject();

		// Check if the method has toObject override
		if(NanHas(object, TO_OBJECT_PROPERTY_NAME)
			&& NanGet(object, TO_OBJECT_PROPERTY_NAME)->IsFunction()) {
				// Get the toObject method
				const Local<Value>& toObject = NanGet(object, TO_OBJECT_PROPERTY_NAME);
				// Call the toObject method
				Local<Value> result = Local<Function>::Cast(toObject)->Call(object, 0, NULL);
				// Did not get an object back
				if(!result->IsObject()) ThrowAllocatedStringException(64, "toObject function did not return an object");
				// Return the value
				return result->ToObject();
		}

		return object;
	}

	case BSON_TYPE_LONG: {
			if((pEnd - p) < 8) {
				ThrowAllocatedStringException(64, "Illegal BSON found, truncated Int64");
			}

			// Read 32 bit integers
			int32_t lowBits = (int32_t) ReadInt32();
			int32_t highBits = (int32_t) ReadInt32();

			// Promote long is enabled
			if(promoteLongs && promoteValues) {
				// If value is < 2^53 and >-2^53
				if((highBits < 0x200000 || (highBits == 0x200000 && lowBits == 0)) && highBits >= -0x200000) {
					// Adjust the pointer and read as 64 bit value
					p -= 8;
					// Read the 64 bit value
					int64_t finalValue = (int64_t) ReadInt64();
					// JS has no int64 type.
					return Nan::New<Number>(static_cast<double>(finalValue));
				}
			}

			// Decode the Long value
			Local<Value> argv[] = { Nan::New<Int32>(lowBits), Nan::New<Int32>(highBits) };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->longConstructor), 2, argv);
			return obj.ToLocalChecked();
		}

	case BSON_TYPE_DATE: {
		if((pEnd - p) < 8) {
			ThrowAllocatedStringException(64, "Illegal BSON found, truncated DateTime");
		}

		return Unmaybe(Nan::New<Date>((double) ReadInt64()));
	}

	case BSON_TYPE_ARRAY:
		return DeserializeArray(raw);

	case BSON_TYPE_OBJECT:
		return DeserializeDocument(raw);

	case BSON_TYPE_SYMBOL: {
			const Local<String>& string = ReadString();
			Local<Value> argv[] = { string };
			Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->symbolConstructor), 1, argv);
			return obj.ToLocalChecked();
		}

	case BSON_TYPE_MIN_KEY: {
		Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->minKeyConstructor));
		return obj.ToLocalChecked();
	}

	case BSON_TYPE_MAX_KEY: {
		Nan::MaybeLocal<Object> obj = Nan::NewInstance(Nan::New(bson->maxKeyConstructor));
		return obj.ToLocalChecked();
	}

	default:
		ThrowAllocatedStringException(64, "Unhandled BSON Type: %d", type);
	}

	return Nan::Null();
}

// statics
Persistent<FunctionTemplate> BSON::constructor_template;
Nan::Persistent<Integer> BSON::indexes[preLoadedIndex];
Nan::Persistent<String> BSON::indexesStrings[preLoadedIndex];
bool BSON::initialized;

BSON::BSON() : ObjectWrap()
{
}

BSON::~BSON()
{
	Nan::HandleScope scope;
	// dispose persistent handles
	buffer.Reset();
	longConstructor.Reset();
	int32Constructor.Reset();
	objectIDConstructor.Reset();
	binaryConstructor.Reset();
	decimalConstructor.Reset();
	codeConstructor.Reset();
	dbrefConstructor.Reset();
	symbolConstructor.Reset();
	doubleConstructor.Reset();
	timestampConstructor.Reset();
	minKeyConstructor.Reset();
	maxKeyConstructor.Reset();

	// Labels
	LONG_CLASS_NAME_STR.Reset();
	LONG_LOW_PROPERTY_NAME_STR.Reset();
	LONG_HIGH_PROPERTY_NAME_STR.Reset();
	TIMESTAMP_CLASS_NAME_STR.Reset();
	OBJECT_ID_CLASS_NAME_STR.Reset();
	OBJECT_ID_ID_PROPERTY_NAME_STR.Reset();
	BINARY_CLASS_NAME_STR.Reset();
	DECIMAL128_CLASS_NAME_STR.Reset();
	DECIMAL128_VALUE_PROPERTY_NAME_STR.Reset();
	DOUBLE_CLASS_NAME_STR.Reset();
	DOUBLE_VALUE_PROPERTY_NAME_STR.Reset();
	INT32_CLASS_NAME_STR.Reset();
	INT32_VALUE_PROPERTY_NAME_STR.Reset();
	SYMBOL_CLASS_NAME_STR.Reset();
	CODE_CLASS_NAME_STR.Reset();
	DBREF_CLASS_NAME_STR.Reset();
	REGEXP_CLASS_NAME_STR.Reset();
	MAX_KEY_CLASS_NAME_STR.Reset();
	MIN_KEY_CLASS_NAME_STR.Reset();
	BINARY_POSITION_PROPERTY_NAME_STR.Reset();
	BINARY_BUFFER_PROPERTY_NAME_STR.Reset();
	BINARY_SUBTYPE_PROPERTY_NAME_STR.Reset();
	SYMBOL_VALUE_PROPERTY_NAME_STR.Reset();
	CODE_CODE_PROPERTY_NAME_STR.Reset();
	CODE_SCOPE_PROPERTY_NAME_STR.Reset();
	DBREF_NAMESPACE_PROPERTY_NAME_STR.Reset();
	DBREF_OID_PROPERTY_NAME_STR.Reset();
	DBREF_DB_PROPERTY_NAME_STR.Reset();
	REGEX_PATTERN_PROPERTY_NAME_STR.Reset();
	REGEX_OPTIONS_PROPERTY_NAME_STR.Reset();
}

void BSON::initializeStatics() {
 	if(!initialized) {
		// Pre-load `indexes` for array serialization/deserialize
	 	for(int32_t index = 0; index < preLoadedIndex; index++) {
	 		indexes[index].Reset(Nan::New<Integer>(index));
	 		indexesStrings[index].Reset(Nan::New<Integer>(index)->ToString());
	 	}

		// Set initialized to true
 		initialized = true;
 	}
}

void BSON::Initialize(v8::Local<v8::Object> target) {
	// Grab the scope of the call from Node
	Nan::HandleScope scope;
	// Define a new function template
	Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);
	t->InstanceTemplate()->SetInternalFieldCount(1);
	t->SetClassName(NanStr("BSON"));

	// Instance methods
	Nan::SetPrototypeMethod(t, "calculateObjectSize", CalculateObjectSize);
	Nan::SetPrototypeMethod(t, "serialize", BSONSerialize);
	Nan::SetPrototypeMethod(t, "serializeWithBufferAndIndex", SerializeWithBufferAndIndex);
	Nan::SetPrototypeMethod(t, "deserialize", BSONDeserialize);
	Nan::SetPrototypeMethod(t, "deserializeStream", BSONDeserializeStream);

	constructor_template.Reset(t);

	target->Set(NanStr("BSON"), t->GetFunction());
}

// Create a new instance of BSON and passing it the existing context
NAN_METHOD(BSON::New) {
	Nan::HandleScope scope;

	// Var maximum bson size
	uint32_t maxBSONSize = MAX_BSON_SIZE;

	// Check that we have an array
	if(info.Length() >= 1 && info[0]->IsArray()) {
		// Cast the array to a local reference
		Local<Array> array = Local<Array>::Cast(info[0]);

		// If we have an options object we can set the maximum bson size to enforce
		if(info.Length() == 2 && info[1]->IsObject()) {
			Local<Object> options = info[1]->ToObject();

			// Do we have a value set
			if(NanHas(options, "maxBSONSize") && NanGet(options, "maxBSONSize")->IsNumber()) {
				maxBSONSize = (size_t)NanGet(options, "maxBSONSize")->Int32Value();
			}
		}

		if(array->Length() > 0) {
			// Create a bson object instance and return it
			BSON *bson = new BSON();
			// Initialize global values
			BSON::initializeStatics();
			// Set max BSON size
			bson->maxBSONSize = maxBSONSize;
			// Create cached labels (avoid translation cost for every lookup)
			bson->LONG_CLASS_NAME_STR.Reset(Nan::New<String>(LONG_CLASS_NAME).ToLocalChecked());
			bson->LONG_LOW_PROPERTY_NAME_STR.Reset(Nan::New<String>(LONG_LOW_PROPERTY_NAME).ToLocalChecked());
			bson->LONG_HIGH_PROPERTY_NAME_STR.Reset(Nan::New<String>(LONG_HIGH_PROPERTY_NAME).ToLocalChecked());
			bson->TIMESTAMP_CLASS_NAME_STR.Reset(Nan::New<String>(TIMESTAMP_CLASS_NAME).ToLocalChecked());
			bson->OBJECT_ID_CLASS_NAME_STR.Reset(Nan::New<String>(OBJECT_ID_CLASS_NAME).ToLocalChecked());
			bson->OBJECT_ID_ID_PROPERTY_NAME_STR.Reset(Nan::New<String>(OBJECT_ID_ID_PROPERTY_NAME).ToLocalChecked());
			bson->BINARY_CLASS_NAME_STR.Reset(Nan::New<String>(BINARY_CLASS_NAME).ToLocalChecked());
			bson->DECIMAL128_CLASS_NAME_STR.Reset(Nan::New<String>(DECIMAL128_CLASS_NAME).ToLocalChecked());
			bson->DECIMAL128_VALUE_PROPERTY_NAME_STR.Reset(Nan::New<String>(DECIMAL128_VALUE_PROPERTY_NAME).ToLocalChecked());
			bson->DOUBLE_CLASS_NAME_STR.Reset(Nan::New<String>(DOUBLE_CLASS_NAME).ToLocalChecked());
			bson->DOUBLE_VALUE_PROPERTY_NAME_STR.Reset(Nan::New<String>(DOUBLE_VALUE_PROPERTY_NAME).ToLocalChecked());
			bson->INT32_CLASS_NAME_STR.Reset(Nan::New<String>(INT32_CLASS_NAME).ToLocalChecked());
			bson->INT32_VALUE_PROPERTY_NAME_STR.Reset(Nan::New<String>(INT32_VALUE_PROPERTY_NAME).ToLocalChecked());
			bson->SYMBOL_CLASS_NAME_STR.Reset(Nan::New<String>(SYMBOL_CLASS_NAME).ToLocalChecked());
			bson->CODE_CLASS_NAME_STR.Reset(Nan::New<String>(CODE_CLASS_NAME).ToLocalChecked());
			bson->DBREF_CLASS_NAME_STR.Reset(Nan::New<String>(DBREF_CLASS_NAME).ToLocalChecked());
			bson->REGEXP_CLASS_NAME_STR.Reset(Nan::New<String>(REGEXP_CLASS_NAME).ToLocalChecked());
			bson->MAX_KEY_CLASS_NAME_STR.Reset(Nan::New<String>(MAX_KEY_CLASS_NAME).ToLocalChecked());
			bson->MIN_KEY_CLASS_NAME_STR.Reset(Nan::New<String>(MIN_KEY_CLASS_NAME).ToLocalChecked());
			bson->BINARY_POSITION_PROPERTY_NAME_STR.Reset(Nan::New<String>(BINARY_POSITION_PROPERTY_NAME).ToLocalChecked());
			bson->BINARY_BUFFER_PROPERTY_NAME_STR.Reset(Nan::New<String>(BINARY_BUFFER_PROPERTY_NAME).ToLocalChecked());
			bson->BINARY_SUBTYPE_PROPERTY_NAME_STR.Reset(Nan::New<String>(BINARY_SUBTYPE_PROPERTY_NAME).ToLocalChecked());
			bson->SYMBOL_VALUE_PROPERTY_NAME_STR.Reset(Nan::New<String>(SYMBOL_VALUE_PROPERTY_NAME).ToLocalChecked());
			bson->CODE_CODE_PROPERTY_NAME_STR.Reset(Nan::New<String>(CODE_CODE_PROPERTY_NAME).ToLocalChecked());
			bson->CODE_SCOPE_PROPERTY_NAME_STR.Reset(Nan::New<String>(CODE_SCOPE_PROPERTY_NAME).ToLocalChecked());
			bson->DBREF_NAMESPACE_PROPERTY_NAME_STR.Reset(Nan::New<String>(DBREF_NAMESPACE_PROPERTY_NAME).ToLocalChecked());
			bson->DBREF_OID_PROPERTY_NAME_STR.Reset(Nan::New<String>(DBREF_OID_PROPERTY_NAME).ToLocalChecked());
			bson->DBREF_DB_PROPERTY_NAME_STR.Reset(Nan::New<String>(DBREF_DB_PROPERTY_NAME).ToLocalChecked());
			bson->REGEX_PATTERN_PROPERTY_NAME_STR.Reset(Nan::New<String>(REGEX_PATTERN_PROPERTY_NAME).ToLocalChecked());
			bson->REGEX_OPTIONS_PROPERTY_NAME_STR.Reset(Nan::New<String>(REGEX_OPTIONS_PROPERTY_NAME).ToLocalChecked());

			// Allocate a new Buffer
			bson->buffer.Reset(Unmaybe(Nan::NewBuffer(sizeof(char) * maxBSONSize)));

			// Defined the classmask
			uint32_t foundClassesMask = 0;

			// Iterate over all entries to save the instantiate functions
			for(uint32_t i = 0; i < array->Length(); i++) {
				// Let's get a reference to the function
				Local<Function> func = Local<Function>::Cast(NanGet(array, i));
				Local<String> functionName = func->GetName()->ToString();

				// Save the functions making them persistant handles (they don't get collected)
				if(functionName->StrictEquals(NanStr(LONG_CLASS_NAME))) {
					bson->longConstructor.Reset(func);
					foundClassesMask |= 1;
				} else if(functionName->StrictEquals(NanStr(OBJECT_ID_CLASS_NAME))) {
					bson->objectIDConstructor.Reset(func);
					foundClassesMask |= 2;
				} else if(functionName->StrictEquals(NanStr(BINARY_CLASS_NAME))) {
					bson->binaryConstructor.Reset(func);
					foundClassesMask |= 4;
				} else if(functionName->StrictEquals(NanStr(CODE_CLASS_NAME))) {
					bson->codeConstructor.Reset(func);
					foundClassesMask |= 8;
				} else if(functionName->StrictEquals(NanStr(DBREF_CLASS_NAME))) {
					bson->dbrefConstructor.Reset(func);
					foundClassesMask |= 0x10;
				} else if(functionName->StrictEquals(NanStr(SYMBOL_CLASS_NAME))) {
					bson->symbolConstructor.Reset(func);
					foundClassesMask |= 0x20;
				} else if(functionName->StrictEquals(NanStr(DOUBLE_CLASS_NAME))) {
					bson->doubleConstructor.Reset(func);
					foundClassesMask |= 0x40;
				} else if(functionName->StrictEquals(NanStr(TIMESTAMP_CLASS_NAME))) {
					bson->timestampConstructor.Reset(func);
					foundClassesMask |= 0x80;
				} else if(functionName->StrictEquals(NanStr(MIN_KEY_CLASS_NAME))) {
					bson->minKeyConstructor.Reset(func);
					foundClassesMask |= 0x100;
				} else if(functionName->StrictEquals(NanStr(MAX_KEY_CLASS_NAME))) {
					bson->maxKeyConstructor.Reset(func);
					foundClassesMask |= 0x200;
				} else if(functionName->StrictEquals(NanStr(REGEXP_CLASS_NAME))) {
					bson->regexpConstructor.Reset(func);
					foundClassesMask |= 0x400;
				} else if(functionName->StrictEquals(NanStr(DECIMAL128_CLASS_NAME))) {
					bson->decimalConstructor.Reset(func);
					foundClassesMask |= 0x800;
				} else if(functionName->StrictEquals(NanStr(INT32_CLASS_NAME))) {
					bson->int32Constructor.Reset(func);
					foundClassesMask |= 0x1000;
				} else {
					v8::String::Utf8Value str(functionName);
				}
			}

			// Check if we have the right number of constructors otherwise throw an error
			if(foundClassesMask != 0x1fff) {
				delete bson;
				return Nan::ThrowError("Missing function constructor for either [Long/ObjectID/Binary/Code/DbRef/Symbol/Double/Timestamp/MinKey/MaxKey/BSONRegExp/Decimal128]");
			} else {
				bson->Wrap(info.This());
				info.GetReturnValue().Set(info.This());
			}
		} else {
			return Nan::ThrowError("No types passed in");
		}
	} else {
		return Nan::ThrowTypeError("First argument passed in must be an array of types");
	}
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

NAN_METHOD(BSON::BSONDeserialize) {
	Nan::HandleScope scope;

	// Fail if the first argument is not a string or a buffer
	if(info.Length() > 1 && !info[0]->IsString() && !node::Buffer::HasInstance(info[0]))
		return Nan::ThrowError("First Argument must be a Buffer or String.");

	// Promote longs
	bool promoteLongs = true;
	bool promoteBuffers = false;
	bool bsonRegExp = false;
	bool promoteValues = true;
	Local<Object> fieldsAsRaw;

	// If we have an options object
	if(info.Length() == 2 && info[1]->IsObject()) {
		Local<Object> options = info[1]->ToObject();

		// Check if we have the promoteLongs variable
		if(NanHas(options, "promoteLongs")) {
			if(NanGet(options, "promoteLongs")->IsBoolean()) {
				promoteLongs = NanGet(options, "promoteLongs")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("promoteLongs argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "promoteBuffers")) {
			if(NanGet(options, "promoteBuffers")->IsBoolean()) {
				promoteBuffers = NanGet(options, "promoteBuffers")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("promoteBuffers argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "bsonRegExp")) {
			if(NanGet(options, "bsonRegExp")->IsBoolean()) {
				bsonRegExp = NanGet(options, "bsonRegExp")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("bsonRegExp argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "promoteValues")) {
			if(NanGet(options, "promoteValues")->IsBoolean()) {
				promoteValues = NanGet(options, "promoteValues")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("promoteValues argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "fieldsAsRaw")) {
			if(NanGet(options, "fieldsAsRaw")->IsObject()) {
				fieldsAsRaw = NanGet(options, "fieldsAsRaw")->ToObject();
			} else {
				return Nan::ThrowError("promoteValues argument must be a boolean");
			}
		}
	}

	// Define pointer to data
	Local<Object> obj = info[0]->ToObject();

	// Unpack the BSON parser instance
	BSON *bson = ObjectWrap::Unwrap<BSON>(info.This());

	// If we passed in a buffer, let's unpack it, otherwise let's unpack the string
	if(node::Buffer::HasInstance(obj)) {
#if NODE_MAJOR_VERSION == 0 && NODE_MINOR_VERSION < 3
		Local<Object> buffer = ObjectWrap::Unwrap<Buffer>(obj);
		char* data = buffer->data();
		size_t length = buffer->length();
#else
		char* data = node::Buffer::Data(obj);
		size_t length = node::Buffer::Length(obj);
#endif

		// Validate that we have at least 5 bytes
		if(length < 5) return Nan::ThrowError("corrupt bson message < 5 bytes long");

		try {
			BSONDeserializer deserializer(bson, data, length, bsonRegExp, promoteLongs, promoteBuffers, promoteValues, fieldsAsRaw);
			info.GetReturnValue().Set(deserializer.DeserializeDocument(false));
		} catch(char* exception) {
			Local<String> error = NanStr(exception);
			free(exception);
			return Nan::ThrowError(error);
		}
	} else {
		// The length of the data for this encoding
		ssize_t len = Nan::DecodeBytes(info[0]);

		// Validate that we have at least 5 bytes
		if(len < 5) return Nan::ThrowError("corrupt bson message < 5 bytes long");

		// Let's define the buffer size
		char* data = (char *)malloc(len);
		if(data == NULL) die("Failed to allocate char buffer for BSON serialization");
		Nan::DecodeWrite(data, len, info[0]);

		try {
			BSONDeserializer deserializer(bson, data, len, bsonRegExp, promoteLongs, promoteBuffers, promoteValues, fieldsAsRaw);
			// deserializer.promoteLongs = promoteLongs;
			Local<Value> result = deserializer.DeserializeDocument(false);
			free(data);
			info.GetReturnValue().Set(result);

		} catch(char* exception) {
			Local<String> error = NanStr(exception);
			free(exception);
			free(data);
			return Nan::ThrowError(error);
		}
	}
}

Local<Object> BSON::GetSerializeObject(const Local<Value>& argValue)
{
	Local<Object> object = argValue->ToObject();

	if(NanHas(object, TO_BSON_PROPERTY_NAME)) {
		const Local<Value>& toBSON = NanGet(object, TO_BSON_PROPERTY_NAME);
		if(!toBSON->IsFunction()) ThrowAllocatedStringException(64, "toBSON is not a function");

		Local<Value> result = Local<Function>::Cast(toBSON)->Call(object, 0, NULL);
		if(!result->IsObject()) ThrowAllocatedStringException(64, "toBSON function did not return an object");
		return result->ToObject();
	} else {
		return object;
	}
}

NAN_METHOD(BSON::BSONSerialize) {
	Nan::HandleScope scope;

	// Default values
	bool checkKeys = false;
	bool serializeFunctions = false;
	bool ignoreUndefined = true;

	// Make sure we have an object as the first argument
	if(info.Length() >= 1 && !info[0]->IsObject()) {
		return Nan::ThrowError("First argument must be a javascript object");
	}

	// Unpack all the options for the parser
	if(info.Length() >= 2 && info[1]->IsObject()) {
		Local<Object> options = info[1]->ToObject();

		// Check if we have the checkKeys variable
		if(NanHas(options, "checkKeys")) {
			if(NanGet(options, "checkKeys")->IsBoolean()) {
				checkKeys = NanGet(options, "checkKeys")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("checkKeys argument must be a boolean");
			}
		}

		// Check if we have the serializeFunctions variable
		if(NanHas(options, "serializeFunctions")) {
			if(NanGet(options, "serializeFunctions")->IsBoolean()) {
				serializeFunctions = NanGet(options, "serializeFunctions")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("serializeFunctions argument must be a boolean");
			}
		}

		// Check if we have the serializeFunctions variable
		if(NanHas(options, "ignoreUndefined")) {
			if(NanGet(options, "ignoreUndefined")->IsBoolean()) {
				ignoreUndefined = NanGet(options, "ignoreUndefined")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("ignoreUndefined argument must be a boolean");
			}
		}
	}

	// Unpack the BSON parser instance
	BSON *bson = ObjectWrap::Unwrap<BSON>(info.This());

	// Calculate the total size of the document in binary form to ensure we only allocate memory once
	// With serialize function
	// bool serializeFunctions = (info.Length() >= 4) && info[3]->BooleanValue();
	char *final = NULL;
	char *serialized_object = NULL;
	size_t object_size;

	try {
		Local<Object> object = bson->GetSerializeObject(info[0]);
		// Get a reference to the buffer *char pointer
		serialized_object = node::Buffer::Data(Nan::New(bson->buffer));

		// Check if we have a boolean value
		BSONSerializer<DataStream> data(bson, checkKeys, serializeFunctions, ignoreUndefined, serialized_object);
		data.SerializeDocument(object);

		// Get the object size
		object_size = data.GetSerializeSize();
		// Copy the correct size
		final = (char *)malloc(sizeof(char) * object_size);
		// Copy to the final
		memcpy(final, serialized_object, object_size);
		// Assign pointer
		serialized_object = final;
	} catch(char *err_msg) {
		free(final);
		Local<String> error = NanStr(err_msg);
		free(err_msg);
		return Nan::ThrowError(error);
	}

	// NewBuffer takes ownership on the memory, so no need to free the final allocation
	Local<Object> buffer = Unmaybe(Nan::NewBuffer(serialized_object, (uint32_t)object_size));
	info.GetReturnValue().Set(buffer);
}

NAN_METHOD(BSON::CalculateObjectSize) {
	Nan::HandleScope scope;

	// Default values
	bool serializeFunctions = false;
	bool ignoreUndefined = true;
	bool checkKeys = false;

	// Make sure we have an object as the first argument
	if(info.Length() >= 1 && !info[0]->IsObject()) {
		return Nan::ThrowError("First argument must be a javascript object");
	}

	// Unpack all the options for the parser
	if(info.Length() >= 2 && info[1]->IsObject()) {
		Local<Object> options = info[1]->ToObject();

		// Check if we have the checkKeys variable
		if(NanHas(options, "checkKeys")) {
			if(NanGet(options, "checkKeys")->IsBoolean()) {
				checkKeys = NanGet(options, "checkKeys")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("checkKeys argument must be a boolean");
			}
		}

		// Check if we have the serializeFunctions variable
		if(NanHas(options, "serializeFunctions")) {
			if(NanGet(options, "serializeFunctions")->IsBoolean()) {
				serializeFunctions = NanGet(options, "serializeFunctions")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("serializeFunctions argument must be a boolean");
			}
		}

		// Check if we have the serializeFunctions variable
		if(NanHas(options, "ignoreUndefined")) {
			if(NanGet(options, "ignoreUndefined")->IsBoolean()) {
				ignoreUndefined = NanGet(options, "ignoreUndefined")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("ignoreUndefined argument must be a boolean");
			}
		}
	}

	// Unpack the BSON parser instance
	BSON *bson = ObjectWrap::Unwrap<BSON>(info.This());
	BSONSerializer<CountStream> countSerializer(bson, checkKeys, serializeFunctions, ignoreUndefined);
	countSerializer.SerializeDocument(info[0]);

	// Return the object size
	info.GetReturnValue().Set(Nan::New<Uint32>((uint32_t) countSerializer.GetSerializeSize()));
}

NAN_METHOD(BSON::SerializeWithBufferAndIndex) {
	Nan::HandleScope scope;

	// Default values
	bool checkKeys = false;
	bool serializeFunctions = false;
	uint32_t index = 0;
	bool ignoreUndefined = true;
	// Object size
	size_t object_size;

	// Make sure we have an object as the first argument
	if(info.Length() >= 1 && !info[0]->IsObject()) {
		return Nan::ThrowError("First argument must be a javascript object");
	}

	// Make sure we have an object as the first argument
	if(info.Length() >= 2 && !node::Buffer::HasInstance(info[1])) {
		return Nan::ThrowError("Second argument must be a Buffer instance");
	}

	// Unpack all the options for the parser
	if(info.Length() >= 3 && info[2]->IsObject()) {
		Local<Object> options = info[2]->ToObject();

		// Check if we have the checkKeys variable
		if(NanHas(options, "checkKeys")) {
			if(NanGet(options, "checkKeys")->IsBoolean()) {
				checkKeys = NanGet(options, "checkKeys")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("checkKeys argument must be a boolean");
			}
		}

		// Check if we have the serializeFunctions variable
		if(NanHas(options, "serializeFunctions")) {
			if(NanGet(options, "serializeFunctions")->IsBoolean()) {
				serializeFunctions = NanGet(options, "serializeFunctions")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("serializeFunctions argument must be a boolean");
			}
		}

		// Check if we have the serializeFunctions variable
		if(NanHas(options, "ignoreUndefined")) {
			if(NanGet(options, "ignoreUndefined")->IsBoolean()) {
				ignoreUndefined = NanGet(options, "ignoreUndefined")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("ignoreUndefined argument must be a boolean");
			}
		}

		// Check if we have the checkKeys variable
		if(NanHas(options, "index")) {
			if(NanGet(options, "index")->IsUint32()) {
				index =  Nan::To<uint32_t>(NanGet(options, "index")).FromJust();
			} else {
				return Nan::ThrowError("index argument must be an integer");
			}
		}
	}

	try {
		BSON *bson = ObjectWrap::Unwrap<BSON>(info.This());

		Local<Object> obj = info[1]->ToObject();
		char* data = node::Buffer::Data(obj);
		size_t length = node::Buffer::Length(obj);

		BSONSerializer<DataStream> dataSerializer(bson, checkKeys, serializeFunctions, ignoreUndefined, data+index);
		dataSerializer.SerializeDocument(bson->GetSerializeObject(info[0]));
		object_size = dataSerializer.GetSerializeSize();

		if(object_size + index > length) return Nan::ThrowError("Serious error - overflowed buffer!!");
	} catch(char *exception) {
		Local<String> error = NanStr(exception);
		free(exception);
		return Nan::ThrowError(error);
	}

	info.GetReturnValue().Set(Nan::New<Uint32>((uint32_t) (index + object_size - 1)));
}

NAN_METHOD(BSON::BSONDeserializeStream) {
	Nan::HandleScope scope;

	// At least 3 arguments required
	if(info.Length() < 5) return Nan::ThrowError("Arguments required (Buffer(data), Number(index in data), Number(number of documents to deserialize), Array(results), Number(index in the array), Object(optional))");

	// If the number of argumets equals 3
	if(info.Length() >= 5) {
		if(!node::Buffer::HasInstance(info[0])) return Nan::ThrowError("First argument must be Buffer instance");
		if(!info[1]->IsUint32()) return Nan::ThrowError("Second argument must be a positive index number");
		if(!info[2]->IsUint32()) return Nan::ThrowError("Third argument must be a positive number of documents to deserialize");
		if(!info[3]->IsArray()) return Nan::ThrowError("Fourth argument must be an array the size of documents to deserialize");
		if(!info[4]->IsUint32()) return Nan::ThrowError("Sixth argument must be a positive index number");
	}

	// If we have 6 arguments
	if(info.Length() == 6 && !info[5]->IsObject()) return Nan::ThrowError("Fifth argument must be an object with options");

	// Define pointer to data
	Local<Object> obj = info[0]->ToObject();
	uint32_t numberOfDocuments = info[2]->Uint32Value();
	uint32_t index = info[1]->Uint32Value();
	uint32_t resultIndex = info[4]->Uint32Value();
	bool promoteLongs = true;
	bool promoteBuffers = false;
	bool bsonRegExp = false;
	bool promoteValues = true;
	Local<Object> fieldsAsRaw;

	// Check for the value promoteLongs in the options object
	if(info.Length() == 6) {
		Local<Object> options = info[5]->ToObject();

		// Check if we have the promoteLongs variable
		if(NanHas(options, "promoteLongs")) {
			if(NanGet(options, "promoteLongs")->IsBoolean()) {
				promoteLongs = NanGet(options, "promoteLongs")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("promoteLongs argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "promoteBuffers")) {
			if(NanGet(options, "promoteBuffers")->IsBoolean()) {
				promoteBuffers = NanGet(options, "promoteBuffers")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("promoteBuffers argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "bsonRegExp")) {
			if(NanGet(options, "bsonRegExp")->IsBoolean()) {
				bsonRegExp = NanGet(options, "bsonRegExp")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("bsonRegExp argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "promoteValues")) {
			if(NanGet(options, "promoteValues")->IsBoolean()) {
				promoteValues = NanGet(options, "promoteValues")->ToBoolean()->Value();
			} else {
				return Nan::ThrowError("promoteValues argument must be a boolean");
			}
		}

		// Check if we have the promoteLongs variable
		if(NanHas(options, "fieldsAsRaw")) {
			if(NanGet(options, "fieldsAsRaw")->IsObject()) {
				fieldsAsRaw = NanGet(options, "fieldsAsRaw")->ToObject();
			} else {
				return Nan::ThrowError("promoteValues argument must be a boolean");
			}
		}
	}

	// Unpack the BSON parser instance
	BSON *bson = ObjectWrap::Unwrap<BSON>(info.This());

	// Unpack the buffer variable
#if NODE_MAJOR_VERSION == 0 && NODE_MINOR_VERSION < 3
	Local<Object> buffer = ObjectWrap::Unwrap<Buffer>(obj);
	char* data = buffer->data();
	size_t length = buffer->length();
#else
	char* data = node::Buffer::Data(obj);
	size_t length = node::Buffer::Length(obj);
#endif

	// Fetch the documents
	Local<Object> documents = info[3]->ToObject();

	BSONDeserializer deserializer(bson, data+index, length-index, bsonRegExp, promoteLongs, promoteBuffers, promoteValues, fieldsAsRaw);
	for(uint32_t i = 0; i < numberOfDocuments; i++) {
		try {
			documents->Set(i + resultIndex, deserializer.DeserializeDocument(false));
		} catch (char* exception) {
		  Local<String> error = NanStr(exception);
			free(exception);
			return Nan::ThrowError(error);
		}
	}

	// Return new index of parsing
	info.GetReturnValue().Set(Nan::New<Uint32>((uint32_t) (index + deserializer.GetSerializeSize())));
}

// Exporting function
extern "C" void init(Local<Object> target) {
	Nan::HandleScope scope;
	BSON::Initialize(target);
}

NODE_MODULE(bson, BSON::Initialize);
