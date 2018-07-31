---
layout: docs
title: Pointer Field
published: true
---

# Pointer Field

## Definition

caf::PdmPtrField<RimWellPath*> m_parentWell;

## Implentation

    CAF_PDM_InitFieldNoDefault(&m_parentWell, "ParentWell", "Parent Well", "", "", "");
