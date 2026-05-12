# `caf::PdmNestedCollection` — design notes

## Context

`caf::PdmNestedCollection` is the base class for tree-shaped PDM containers — a
folder of items that can also contain folders of the same kind. The design is a
non-template concrete base + a CRTP template:

```cpp
class PdmNestedCollectionBase : public PdmObject
{
    QString            collectionName() const;
    void               setCollectionName( const QString& name );
    virtual bool       canAddSubCollection() const;
    virtual PdmObject* addNewSubCollection();
    void               setAsTopmostFolder();
    PdmField<QString>  m_collectionName;
};

template <typename SelfT, typename ItemT>
class PdmNestedCollection : public PdmNestedCollectionBase
{
    // typed accessors + storage for items and subcollections
    PdmChildArrayField<SelfT*> m_subCollections;
    PdmChildArrayField<ItemT*> m_items;
};
```

`PdmNestedCollectionBase` is the shared PdmObject ancestor — the anchor point
where the CAF method factory can register generic script methods (like
`AddFolder`) once and have them surface on every derived nested collection via
the inheritance walk and Python class hierarchy.

`PdmNestedCollection<SelfT, ItemT>` adds typed storage and accessors. `SelfT` is
the CRTP self type (so subcollections are typed `SelfT*`); `ItemT` is the leaf
item type. The template does not inherit `PdmObjectCollection<ItemT>` — m_items
is declared here directly, so the inheritance chain stays linear and free of
diamond shapes.

## Why both a non-template base and a CRTP template

Two needs that don't fit into a single layer:

- **CAF method registration is keyed by `classKeyword`** — only a concrete
  non-template class can serve as the anchor. Hence the non-template
  `PdmNestedCollectionBase`. Without it, generic script methods (`AddFolder`)
  would have to be re-registered per derived class.

- **Typed storage and accessors** — `m_subCollections : PdmChildArrayField<SelfT*>`
  needs `SelfT`, which only exists as a template parameter. Hence the CRTP
  template layer on top of the base.

## Alternative considered: non-templated base only

```cpp
class PdmNestedCollection : public PdmObject
{
    ...
    virtual PdmObject* createSubCollection() const = 0;
    // m_subCollectionsField / m_itemsField stored as type-erased handles
};
```

Each derived class would own its typed `PdmChildArrayField<DerivedType*>
m_subCollections` and `PdmChildArrayField<ItemType*> m_items`, register them
with the base in the constructor, and override `createSubCollection()`. About
~25 lines of boilerplate per derived class beyond what the current design needs:
two field declarations, two `registerXxxField` calls, an override of
`createSubCollection`, re-declared typed accessors.

At the projected 8–10 derived classes that's ~200–250 lines of repeated code.
The hybrid (non-template base + template) design pays the cost once in the
framework and gets typed access plus generic registration for free.

## When to revisit

- If the count of nested-collection types drops to 1–2 — the template tier no
  longer earns its keep, and a pure non-template base wins on simplicity.
- If template instantiation cost shows up in build profiling for the PDM layer.
- If the typed accessors are rarely used in practice and most call sites
  immediately cast to `PdmObject*` anyway.
