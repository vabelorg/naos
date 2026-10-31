# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# Checks the relative links of the Markdown files of Naos: every linked file must exist and every
# #anchor must name a heading (as GitHub makes its id) or an <a id="..."> of the file linked.
# Links with a scheme (https:, mailto:) are not checked.
#
# -D ROOT=<path>   root of Naos

cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED ROOT)
    message(FATAL_ERROR "check-links.cmake: -D ROOT=<path> is required")
endif()

file(GLOB top_files LIST_DIRECTORIES false "${ROOT}/*.md")
file(GLOB_RECURSE nested_files LIST_DIRECTORIES false
    "${ROOT}/docs/*.md"
    "${ROOT}/examples/*.md"
    "${ROOT}/.github/*.md"
)
set(files ${top_files} ${nested_files})
list(SORT files)

# Sets out to the content of file without its fenced code blocks, ';' made ',' so that matches can
# go through lists.
function(naos_doc_text out file)
    file(READ "${file}" text)
    string(REPLACE "\r" "" text "${text}")
    string(REPLACE ";" "," text "${text}")
    string(REGEX REPLACE "```[^`]*```" "" text "${text}")
    set(${out} "\n${text}" PARENT_SCOPE)
endfunction()

# Sets out to the id GitHub gives the heading text: its words, lowercase, without punctuation and
# with '-' for each space.
function(naos_heading_id out heading)
    string(REGEX REPLACE "\\[([^]]*)\\]\\([^)]*\\)" "\\1" id "${heading}")
    string(REPLACE "[" "" id "${id}")
    string(REPLACE "]" "" id "${id}")
    string(REPLACE "\\" "" id "${id}")
    string(TOLOWER "${id}" id)
    foreach(mark IN ITEMS "!" "\"" "#" "$" "%" "&" "'" "(" ")" "*" "+" "," "." "/" ":" "<" "=" ">"
            "?" "@" "^" "`" "{" "|" "}" "~"
            "（" "）" "「" "」" "『" "』" "、" "。" "・" "：" "，" "！" "？" "／" "【" "】" "〜"
            "—" "–" "→" "…" "‘" "’" "“" "”")
        string(REPLACE "${mark}" "" id "${id}")
    endforeach()
    string(REPLACE " " "-" id "${id}")
    set(${out} "${id}" PARENT_SCOPE)
endfunction()

# Sets out to the anchors of the Markdown file: the ids of its headings, numbered as GitHub numbers
# repeated ones, and those of its <a id="..."> and <a name="...">.
function(naos_doc_anchors out file)
    naos_doc_text(text "${file}")
    set(anchors "")
    string(REGEX MATCHALL "\n#+ [^\n]*" headings "${text}")
    foreach(heading IN LISTS headings)
        string(REGEX REPLACE "^\n#+ +" "" heading "${heading}")
        string(REGEX REPLACE "[ #]+$" "" heading "${heading}")
        naos_heading_id(id "${heading}")
        set(unique "${id}")
        set(count 0)
        while("${unique}" IN_LIST anchors)
            math(EXPR count "${count} + 1")
            set(unique "${id}-${count}")
        endwhile()
        list(APPEND anchors "${unique}")
    endforeach()
    string(REGEX MATCHALL "<a (id|name)=\"[^\"]+\"" tags "${text}")
    foreach(tag IN LISTS tags)
        string(REGEX REPLACE "^<a (id|name)=\"([^\"]+)\"$" "\\2" id "${tag}")
        list(APPEND anchors "${id}")
    endforeach()
    set(${out} "${anchors}" PARENT_SCOPE)
endfunction()

set(errors "")
set(link_count 0)

foreach(file IN LISTS files)
    file(RELATIVE_PATH shown "${ROOT}" "${file}")
    get_filename_component(dir "${file}" DIRECTORY)
    naos_doc_text(text "${file}")
    # Inline code is no link. A ']' without its '[' would keep a CMake list from splitting, so the
    # start of each link target is marked with a word instead, and the brackets dropped.
    string(REGEX REPLACE "`[^`\n]*`" "" text "${text}")
    string(REPLACE "](" " NAOS_LINK(" text "${text}")
    string(REPLACE "[" "" text "${text}")
    string(REPLACE "]" "" text "${text}")
    string(REGEX MATCHALL " NAOS_LINK\\([^) \n]+\\)" links "${text}")

    foreach(link IN LISTS links)
        string(REGEX REPLACE "^ NAOS_LINK\\((.*)\\)$" "\\1" target "${link}")
        if(target MATCHES "^[a-z]+:")
            continue()
        endif()
        math(EXPR link_count "${link_count} + 1")

        string(FIND "${target}" "#" hash)
        if(hash EQUAL -1)
            set(path "${target}")
            set(anchor "")
        else()
            string(SUBSTRING "${target}" 0 ${hash} path)
            math(EXPR anchor_start "${hash} + 1")
            string(SUBSTRING "${target}" ${anchor_start} -1 anchor)
        endif()

        if(path STREQUAL "")
            set(linked "${file}")
        else()
            get_filename_component(linked "${dir}/${path}" ABSOLUTE)
        endif()
        if(NOT EXISTS "${linked}")
            list(APPEND errors "${shown}: no file for the link to '${target}'")
            continue()
        endif()

        if((NOT anchor STREQUAL "") AND linked MATCHES "\\.md$")
            string(MAKE_C_IDENTIFIER "${linked}" key)
            if(NOT DEFINED anchors_${key})
                naos_doc_anchors(anchors_${key} "${linked}")
            endif()
            if(NOT "${anchor}" IN_LIST anchors_${key})
                list(APPEND errors "${shown}: no anchor for the link to '${target}'")
            endif()
        endif()
    endforeach()
endforeach()

list(LENGTH files file_count)
list(LENGTH errors error_count)
if(error_count GREATER 0)
    list(JOIN errors "\n" report)
    message(FATAL_ERROR "${error_count} broken links in ${file_count} files:\n${report}")
endif()
message(STATUS "${link_count} links checked in ${file_count} files")
