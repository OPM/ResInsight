"""
Tests for PdmObjectBase.__makelist() and __convert_from_grpc_value() parsing.

These tests verify quote-aware parsing of gRPC string values, including
strings containing commas, escaped quotes, nested lists, and schedule text.
Regression tests for #13515.
"""

import sys
import os
import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from rips.pdmobject import PdmObjectBase


@pytest.fixture
def parser():
    """Create a PdmObjectBase instance for testing parsing methods."""
    return PdmObjectBase(pb2_object=None, channel=None)


class TestEmptyAndSimpleLists:
    """Tests for basic list parsing."""

    def test_empty_list(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value("[]")
        assert result == []

    def test_single_int(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value("[42]")
        assert result == [42]

    def test_int_list(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value("[1, 2, 3]")
        assert result == [1, 2, 3]

    def test_float_list(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value("[1.5, 2.5, 3.5]")
        assert result == [1.5, 2.5, 3.5]

    def test_bool_list(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value("[true, false, true]")
        assert result == [True, False, True]

    def test_simple_string_list(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value('["hello", "world"]')
        assert result == ["hello", "world"]


class TestStringsWithCommas:
    """Core regression tests for #13515: strings containing commas."""

    def test_single_string_with_comma(self, parser):
        # C++ sends: ["hello, world"]
        result = parser._PdmObjectBase__convert_from_grpc_value('["hello, world"]')
        assert result == ["hello, world"]

    def test_multiple_strings_with_commas(self, parser):
        # C++ sends: ["a, b", "c, d"]
        result = parser._PdmObjectBase__convert_from_grpc_value('["a, b", "c, d"]')
        assert result == ["a, b", "c, d"]

    def test_string_with_multiple_commas(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value(
            '["one, two, three, four"]'
        )
        assert result == ["one, two, three, four"]

    def test_mixed_strings_with_and_without_commas(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value(
            '["no comma", "has, comma", "also none"]'
        )
        assert result == ["no comma", "has, comma", "also none"]


class TestEscapedCharacters:
    """Tests for escaped quotes and backslashes inside strings."""

    def test_escaped_quote_in_string(self, parser):
        # C++ sends: ["say \"hello\""]  -> Python should get: say "hello"
        result = parser._PdmObjectBase__convert_from_grpc_value('["say \\"hello\\""]')
        assert result == ['say "hello"']

    def test_escaped_backslash_in_string(self, parser):
        # C++ sends: ["path\\to\\file"]  -> Python should get: path\to\file
        result = parser._PdmObjectBase__convert_from_grpc_value(
            '["path\\\\to\\\\file"]'
        )
        assert result == ["path\\to\\file"]

    def test_escaped_quote_and_comma(self, parser):
        # C++ sends: ["say \"hi\", bye"]  -> Python should get: say "hi", bye
        result = parser._PdmObjectBase__convert_from_grpc_value('["say \\"hi\\", bye"]')
        assert result == ['say "hi", bye']

    def test_scalar_string_unescape(self, parser):
        # A scalar (non-list) string with escaped characters
        result = parser._PdmObjectBase__convert_from_grpc_value('"say \\"hello\\""')
        assert result == 'say "hello"'

    def test_scalar_string_unescape_backslash(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value(
            '"C:\\\\Users\\\\file.txt"'
        )
        assert result == "C:\\Users\\file.txt"


class TestNestedLists:
    """Tests for nested list parsing."""

    def test_nested_int_lists(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value("[[1, 2], [3, 4]]")
        assert result == [[1, 2], [3, 4]]

    def test_nested_string_lists_with_commas(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value(
            '[["a, b", "c"], ["d", "e, f"]]'
        )
        assert result == [["a, b", "c"], ["d", "e, f"]]


class TestMixedTypes:
    """Tests for lists with mixed types."""

    def test_mixed_type_list(self, parser):
        result = parser._PdmObjectBase__convert_from_grpc_value(
            '[1, "text, with comma", 3.5, true]'
        )
        assert result == [1, "text, with comma", 3.5, True]


class TestScheduleTextRegression:
    """Regression test for schedule text containing COMPDAT with commas."""

    def test_schedule_text_single_string_not_split(self, parser):
        # Schedule text is a single string containing many commas
        # C++ wraps it in quotes: ["COMPDAT\n 'Well' 1  2  3 /\n/"]
        schedule = "COMPDAT\n 'WellA' 10 20 30 40 OPEN 1* 0.1 4* 0.5 /\n/"
        grpc_value = '["' + schedule.replace("\\", "\\\\").replace('"', '\\"') + '"]'
        result = parser._PdmObjectBase__convert_from_grpc_value(grpc_value)
        assert len(result) == 1
        assert result[0] == schedule

    def test_compdat_with_commas_in_comments(self, parser):
        # A COMPDAT string that contains commas in inline comments
        text = "COMPDAT -- well completions, zone A, zone B\n 'Well' 1 2 3 /\n/"
        grpc_value = '["' + text.replace("\\", "\\\\").replace('"', '\\"') + '"]'
        result = parser._PdmObjectBase__convert_from_grpc_value(grpc_value)
        assert len(result) == 1
        assert result[0] == text
