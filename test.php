<?php

const CXTranslationUnit_DetailedPreprocessingRecord = 0x01;
const CXTranslationUnit_KeepGoing = 0x200;
const CXTranslationUnit_SingleFileParse = 0x400;

const CXChildVisit_Break = 0;
const CXChildVisit_Continue = 1;
const CXChildVisit_Recurse = 2;

$index = clang_createIndex(false, false);
var_export($index);

$options = CXTranslationUnit_DetailedPreprocessingRecord;
$unit = clang_parseTranslationUnit($index, null, [ "/Users/oscar/Projects/zircon/kernel/arch/x86/mmu.cpp", '-I/Users/oscar/Projects/zircon/kernel/arch/x86/include' ], [], $options);
var_export($unit);

$cursor = clang_getTranslationUnitCursor($unit);
var_export($cursor);

clang_visitChildren($cursor, function($cursor, $parentCursor) {
    $extent = clang_getCursorExtent($cursor);
    $start = clang_getRangeStart($extent);
    $end = clang_getRangeEnd($extent);

    clang_getInstantiationLocation($start, $startFile, $startLine, $startColumn, $startOffset);
    clang_getInstantiationLocation($end, $endFile, $endLine, $endColumn, $endOffset);

    $cursorType = clang_getCursorType($cursor);

    echo json_encode([
        'start' => [
            'line' => $startLine,
            'column' => $startColumn,
            'fileName' => $startFile ? clang_getFileName($startFile) : null,
        ],
        'end' => [
            'line' => $endLine,
            'column' => $endColumn,
            'fileName' => $endFile ? clang_getFileName($endFile) : null,
        ],
        'cursorSpelling' => clang_getCursorSpelling($cursor),
        'USR' => clang_getCursorUSR($cursor),
        'cursorKind' => ($cursorKind = clang_getCursorKind($cursor)),
        'cursorKindSpelling' => clang_getCursorKindSpelling($cursorKind),
        'cursorTypeKind' => $cursorType->kind,
        'cursorTypeSpelling' => clang_getTypeSpelling($cursorType),
        'cursorDisplayName' => clang_getCursorDisplayName($cursor),
        'isDeclaration' => clang_isDeclaration($cursorKind),
        'isReference' => clang_isReference($cursorKind),
        'isExpression' => clang_isExpression($cursorKind),
        'isStatement' => clang_isStatement($cursorKind),
        'isAttribute' => clang_isAttribute($cursorKind),
        'isInvalid' => clang_isInvalid($cursorKind),
        'isTranslationUnit' => clang_isTranslationUnit($cursorKind),
        'isPreprocessing' => clang_isPreprocessing($cursorKind),
        'isUnexposed' => clang_isUnexposed($cursorKind),
    ], JSON_PRETTY_PRINT).PHP_EOL;

    return CXChildVisit_Recurse;
});
