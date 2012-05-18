

namespace caf
{



class PdmFieldHandle
{
    // ....

    void removeFieldView(PdmUiFieldViewHandle* fieldView)   { m_fieldViews.erase(fieldView); }
    void addFieldView(PdmUiFieldViewHandle* fieldView)      { m_fieldViews.insert(fieldView); }
private:
    std::set<PdmUiFieldViewHandle*> m_fieldViews;
    // ....
    void setValueFromUI(...) 
    { 
        //... 
        std::set<PdmUiFieldViewHandle*>::iterator it;
        for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
        {
            m_fieldViews[i]->updateUiValue();
        }

        //...
    } 

};



class PdmUiItemInfo
{
 // ....
    QString m_editorType; // Int, type_info, className or ??
    int     m_isHidden;
    int     m_isReadOnly;

};




class PdmUiItem
{
    // ...

    QString editorType(const QString& uiConfigName);
    void    setEditorType(const QString& uiConfigName, const QString& editorKeyword);

    bool    isHidden(QString uiConfigName);
    void    setHidden(QString uiConfigName, bool isHidden);

    bool    readOnly(QString uiConfigName);
    void    setReadOnly(QString uiConfigName, bool isReadOnly);

    virtual bool isGroup() { return false; }

private :
    /// Map containing the UiItemInfo's for the different UiConfigurations
    /// Each UiItemInfo member is undefined until set. If the member is undefined, we use the default
    /// settings for that parameter.
    std::map< QString, PdmUiItemInfo > m_uiConfigurations; // Empty config name is replacing m_dynamicItemInfo

};




class PdmUiConfiguration
{
public:
    PdmUiConfiguration(): m_forgetRemainingFields(false) { };
    virtual ~PdmUiConfiguration() 
    {
        for (size_t i = 0; i < m_createdGroups.size(); ++i)
        {
            delete m_createdGroups[i];
            m_createdGroups[i] = NULL;
        }
    }

    PdmUiGroup* addNewGroup(QString displayName)
    {
        PdmUiGroup* group = new PdmUiGroup;
        group->setUiName(displayName);

        m_createdGroups.push_back(group);
        m_config.push_back(group);
    }

    void                           add(PdmUiItem* item)               { m_config.push_back(item); }
    bool                           forgetRemainingFields() const      { return m_forgetRemainingFields; }
    void                           setForgetRemainingFields(bool val) { m_forgetRemainingFields = val; }

    const std::vector<PdmUiItem*>& uiItems() const                    { return m_config; }

private:
    // Private copy constructor and assignment to prevent this. (The vectors below will make trouble)
    PdmUiConfiguration(const PdmUiConfiguration& other)                                 { }
    PdmUiConfiguration&       operator= (const PdmUiConfiguration& other)               { }

    std::vector<PdmUiItem*>     m_config;
    std::vector<PdmUiGroup*>    m_createdGroups; /// Owned PdmUiGroups, for mem management
    bool                        m_forgetRemainingFields;

};




class PdmUiGroup : public PdmUiItem, PdmUiConfiguration
{
    virtual bool isGroup() { return true; }
};




class PdmObject : public PdmUiItem
{
public:

    /// For a specific field, return editor specific parameters used to customize the editor behavior..
    virtual void setUpUiAttribute(const PdmFieldHandle* field, QString uiConfigName, PdmUiAttributeHandle * attributes);
   // For later // virtual void uiAttributeChangedByUI(const PdmFieldHandle* field, QString uiConfigName, const PdmUiAttributeHandle * attributes);

    // Method to be called from the Ui classes creating Auto Gui to get the group information 
    // supplied by the \sa setUpUIConfiguration method that can be reimplemented

    void uiConfiguration(QString uiConfigName, PdmUiConfiguration& uiConfig)
    {
        this->setUpUIConfiguration(uiConfigName, uiConfig);
        if (!uiConfig.forgetRemainingFields())
        {
            // Todo: add Remaining Fields To UiConfig


        }
    }

protected:
    /// Fill up the UiConfig object with groups and field references to create the gui structure
    /// If the uiConfig is empty, it is interpreted as meaning all fields w/o grouping.

    virtual void setUpUIConfiguration(QString uiConfigName, PdmUiConfiguration& uiConfig) ;

};




class PdmPropertyWindow : public QWidget
{
public:
    PdmPropertyWindow()
    {
        setLayout(new QHBoxLayout());
    }

    ~PdmPropertyWindow()
    {
        if (m_currentObjectView) delete m_currentObjectView;
    }

    void setUiConfigurationName(QString uiConfigName)
    {
        // Reset everything, and possibly create widgets etc afresh
        if (m_uiConfigName != uiConfigName)
        { 
            m_uiConfigName = uiConfigName;

            if (m_currentObjectView)
            {
                PdmObject* object = m_currentObjectView->pdmObject();
                delete m_currentObjectView;
                m_currentObjectView = NULL;
                this->showProperties(object);
            }
        }
    }

    void showProperties(const PdmObject* object)
    {
        // Find specialized object view handle 

        // If the current ObjectView has the same type as the one to view, reuse, with Widget etc.

        if (!(m_currentObjectView && m_currentObjectView->m_pdmObject->editorType(m_uiConfigName) == object->editorType(m_uiConfigName)))
        {
            // Remove Widget from layout
            layout()->removeWidget(m_currentObjectView->widget());
            delete m_currentObjectView;

            m_currentObjectView = PdmObjViewFactory::instance()->create(object->editorType(m_uiConfigName));
            if (!m_currentObjectView)
            {
                m_currentObjectView = new PdmStdObjView();
            }

            // Create widget to handle this
            QWidget * page = NULL;
            page = m_currentObjectView->getOrCreateWidget(this);

            CVF_ASSERT(page);

            this->layout()->addWidget(page);
        }

        m_currentObjectView->setPdmObject(object);
        m_currentObjectView->updateUi(m_uiConfigName);
    }

private:
    PdmObjectViewHandle* m_currentObjectView; 
    QString              m_uiConfigName;
};




class PdmObjectViewHandle: public QObject
{
public:
    PdmObjectViewHandle() : m_pdmObject(NULL) {}
    ~PdmObjectViewHandle() {}
    /// 
    QWidget* getOrCreateWidget(QWidget* parent) 
    { 
        if (m_widget.isNull()) 
        {
            m_widget = this->createWidget(parent);
        }
        return m_widget; 
    }
    QWidget*            widget() { return m_widget; }

    /// Virtual method to be overridden. Needs to set up the supplied widget
    /// with all signals etc to make it communicate with this object
    void                setPdmObject(PdmObject* object, QString uiConfigName) { m_pdmObject = object;  }
    PdmObject*          pdmObject()                     { return m_pdmObject; }

    virtual void        updateUi(QString uiConfigName) = 0;

protected:
   
    virtual QWidget*    createWidget(QWidget* parent) = 0;

private:
    PdmObject*          m_pdmObject;
    QPointer<QWidget>   m_widget;
};



class PdmUiFieldViewHandle : public QObject
{
public:
    PdmUiFieldViewHandle() : m_field(NULL) {}
    ~PdmUiFieldViewHandle() 
    { 
        if (m_field) m_field->removeFieldView(this); 

        if (!m_combinedWidget.isNull()) delete m_combinedWidget; 
        if (!m_editorWidget.isNull()) delete m_editorWidget ;
        if (!m_labelWidget.isNull()) delete m_labelWidget;
    }

    /// 
    PdmFieldHandle* field() { return m_field; } 
   void             setField(PdmFieldHandle * field) 
    {
        if (m_field) m_field->removeFieldView(this);
        m_field = field; 
        if (m_field) m_field->addFieldView(this);
    }


    void            setValueToField(const QVariant& value)
    {
        if (m_field) m_field->setUiValue(value);
    }

    void            createWidgets(QWidget * parent)
    {
        if (m_combinedWidget.isNull()) m_combinedWidget = createCombinedWidget(parent);
        if (m_editorWidget.isNull()) m_editorWidget = createEditorWidget(parent);
        if (m_labelWidget.isNull()) m_labelWidget = createLabelWidget(parent);
    }

    QWidget* combinedWidget()     { return m_combinedWidget; }
    QWidget* editorWidget()       { return m_editorWidget; }
    QWidget* labelWidget()        { return m_labelWidget; }

public: // Virtual interface to override
    /// Update only the display of the data value, because some other view has changed it.
    virtual void    updateUiValue() = 0;

    /// Supposed to update all parts of the widgets, both visibility, sensitivity, decorations and field data
    virtual void    updateUi(QString uiConfigName) = 0;

    /// Supposed to do all wiring of singals and slots
    virtual void    connectUi() = 0;

protected: // Virtual interface to override
    /// Implement one of these, or both editor and label. The widgets will be used in the parent layout according to 
    /// being "Label" Editor" or a single combined widget. 

    virtual QWidget* createCombinedWidget(QWidget * parent) { return NULL; }
    virtual QWidget* createEditorWidget(QWidget * parent)   { return NULL; }
    virtual QWidget* createLabelWidget(QWidget * parent)    { return NULL; }

private:
    PdmFieldHandle* m_field;

    QPointer<QWidget> m_combinedWidget;
    QPointer<QWidget> m_editorWidget;
    QPointer<QWidget> m_labelWidget;
};



class PdmStdObjView : PdmObjectViewHandle
{
public:
    PdmStdObjView()  {};
    ~PdmStdObjView() {}

protected:

    virtual QWidget* createWidget(QWidget* parent)
    {
        m_mainWidget = new QWidget(parent);
        m_layout = new QGridLayout();
        m_mainWidget->setLayout(m_layout);
        return m_mainWidget;
    }

    virtual void updateUi(QString uiConfigName)
    {
        PdmUiConfiguration config;
        m_pdmObject->uiConfiguration(uiConfigName, &config);

        // Set all fieldViews to be unvisited
        std::map<QString, PdmFieldViewHandle*>::iterator it;
        for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
        {
            it->second->setField(NULL);
        }

        // Set all group Boxes to be unvisited
        m_newGroupBoxes.clear();

        const std::vector<PdmUiItem*>& uiItems = config.uiItems();

        recursiveSetupFieldsAndGroups(uiItems, m_mainWidget, m_layout, uiConfigName);

        // Remove all fieldViews not mentioned by the configuration from the layout

        std::map<QString, PdmFieldViewHandle*>::iterator it;
        std::vector< QString > fvhToRemoveFromMap;
        for (it = m_fieldViews.begin(); it != m_fieldViews.end(); ++it)
        {
            if (it->second->field() == 0)
            {
                PdmFieldViewHandle* fvh = it->second;
                delete fvh;
                fvhToRemoveFromMap.push_back(it->first);               
            }
        }

        for (size_t i = 0; i < fvhToRemoveFromMap.size(); ++i)
        {
            m_fieldViews.erase(fvhToRemoveFromMap[i]);
        }

        // Remove all unmentioned group boxes

        std::map<QString, QPointer<QGroupBox> >::iterator itOld;
        std::map<QString, QPointer<QGroupBox> >::iterator itNew;

        for (itOld = m_groupBoxes.begin(); itOld != m_groupBoxes.end(); ++itOld )
        {
            itNew = m_newGroupBoxes.find(itOld->first);
            if (itNew == m_newGroupBoxes.end()) 
            {
                // The old groupBox is not present anymore, get rid of it
                if (!itOld->second.isNull()) delete itOld->second;
            }
        }
        m_groupBoxes = m_newGroupBoxes;
    }

    void recursiveSetupFieldsAndGroups(const std::vector<PdmUiItem*>& uiItems, QWidget* parent, QGridLayout* parentLayout, const QString& uiConfigName )
    {
        int currentRowIndex = 0;
        for (size_t i = 0; i < uiItems.size(); ++i)
        {
            if (uiItems[i].isGroup())
            {
                const std::vector<PdmUiItem*>& groupChildren = uiItems[i]->uiItems();
                QString groupBoxKey = uiItems[i]->uiName();
                QGroupBox* groupBox = NULL;
                QGridLayout* groupBoxLayout = NULL;

                // Find or create groupBox
                std::map<QString, QPointer<QGroupBox> >::iterator it;
                it = m_groupBoxes.find(groupBoxKey);

                if (it == m_groupBoxes.end())
                {
                    groupBox = new QGroupBox( parent );
                    groupBox->setTitle(uiItems[i]->uiName());

                    m_newGroupBoxes[groupBoxKey] = groupBox;
                    groupBoxLayout = new QGridLayout();
                    groupBox->setLayout(groupBoxLayout);
                }
                else
                {
                    groupBox = it->second;
                    CVF_ASSERT(groupBox);
                    groupBoxLayout = dynamic_cast<QGridLayout*>(groupBox->layout());
                    CVF_ASSERT(groupBoxLayout);
                    m_newGroupBoxes[uiItems[i]->uiName()];
                }
                
                /// Insert the group box at the correct position of the parent layout

                parentLayout->addWidget(groupBox, currentRowIndex, 0, 1, 2);
                recursiveSetupFieldsAndGroups(groupChildren, groupBox, groupBoxLayout);
                currentRowIndex++;
            }
            else
            {
                PdmFieldHandle* field = dynamic_cast<PdmFieldHandle*>(uiItems[i]);
                PdmUiFieldViewHandle* fvh = NULL;

                if (!field->isHidden(uiConfName))
                {

                    // Find or create FieldView
                    std::map<QString, PdmUiFieldViewHandle*>::iterator it;
                    it = m_fieldViews.find(field->keyword());

                    if (it == m_fieldViews.end())
                    {
                        if ( uiItems[i]->editorType(uiConfigName).isDefined() )
                        {
                            fvh = PdmFieldViewFactory::instance()->create( uiItems[i]->editorType(uiConfigName));

                        }
                        else
                        {
                             fvh = PdmFieldViewFactory::instance()->create(typeid(uiItems[i]));
                        }
                        m_fieldViews[field->keyword()] = fvh;
                        fvh->createWidgets(parent);
                    }
                    else
                    {
                        fvh = it->second;
                    }

                    CVF_ASSERT(fvh);

                    fvh->setField(field); 

                    // Place the widget(s) into the correct parent and layout
                    QWidget* fieldCombinedWidget  = fvh->combinedWidget();

                    if (combinedWidget)
                    {
                        combinedWidget->setParent(parent);
                        parentLayout->addWidget(combinedWidget, currentRowIndex, 0, 1, 2);
                    }
                    else
                    {
                        QWidget* fieldEditorWidget = fvh->editorWidget();
                        QWidget* fieldLabelWidget  = fvh->labelWidget();

                        if (fieldEditorWidget)
                        {
                            fieldEditorWidget->setParent(parent); // To make sure this widget has the current group box as parent.
                            parentLayout->addWidget(fieldEditorWidget, currentRowIndex, 1);
                        }

                        if (fieldLabelWidget)
                        {
                            fieldLabelWidget->setParent(parent);
                            parentLayout->addWidget(fieldLabelWidget, currentRowIndex, 0);
                        }
                    }
                   
                    fvh->updateUi(uiConfigName);
                    currentRowIndex++;
                }
            }
        }
    }

private:

    std::map<QString, PdmUiFieldViewHandle*>    m_fieldViews; 
    std::map<QString, QPointer<QGroupBox> >     m_groupBoxes;
    std::map<QString, QPointer<QGroupBox> >     m_newGroupBoxes; ///< used temporarily to store the new(complete) set of group boxes

    QPointer<QWidget>                           m_mainWidget
    QGridLayout*                                m_layout;
};



caf::Factory<PdmUiFieldViewHandle, type_info>::instance()->registerCreator<PdmLineEditFieldView>(typeid(PdmField<QString>));
caf::Factory<PdmUiFieldViewHandle, type_info>::instance()->registerCreator<PdmLineEditFieldView>(typeid(PdmField<int>));
caf::Factory<PdmUiFieldViewHandle, type_info>::instance()->registerCreator<PdmLineEditFieldView>(typeid(PdmField<double>));
caf::Factory<PdmUiFieldViewHandle, type_info>::instance()->registerCreator<PdmLineEditFieldView>(typeid(PdmField<size_t>));

caf::Factory<PdmUiFieldViewHandle, type_info>::instance()->registerCreator<PdmLineEditFieldView>(typeid(PdmUiFileEditor));

class PdmUiAttributeHandle
{
public:
    PdmUiAttributeHandle() {}
    virtual ~PdmUiAttributeHandle() {} 
};

class PdmLineEditAttribute : public PdmUiAttributeHandle
{
    
};

class PdmLineEditFieldView : public PdmUiFieldViewHandle
{
public:
    PdmLineEditFieldView()          {} 
    virtual ~PdmLineEditFieldView() {} 


    virtual void    updateUiValue()
    {
        m_lineEdit->setText(m_field->uiValue().asString());
    };

    virtual void updateUi(const QString& uiConfigName)
    {
        CVF_ASSERT(!m_lineEdit.isNull());
        CVF_ASSERT(!m_label.isNull());

        QIcon ic = m_field->uiIcon(uiConfigName);
        if (!ic.isNull())
        {
            m_label->setPixmap(ic->pixmap());
        }
        else
        {
            m_label->setText(m_field->uiName(uiConfigName));
        }

        m_label->show( !m_field->isHidden(uiConfigName));

        m_lineEdit->setEnabled(!m_field->readOnly(uiConfigName));
        m_label->setEnabled(!m_field->readOnly(uiConfigName));

        PdmLineEditAttribute leab;
        m_field->ownerObject()->setUpUiAttribute(m_field, uiConfigName, &leab);

        if (dynamic_cast<PdmField<int>*> (m_field)) 
        {
            m_lineEdit->setValidator(QIntValidator());
        }

        m_lineEdit->setAlignment(leab.alignment);
    }

    virtual void connectUi()
    {
        connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    }

protected:
    virtual QWidget* createEditorWidget(QWidget * parent)
    {
        m_lineEdit = new QLineEdit(parent);
        return m_lineEdit;
    }

    virtual QWidget* createLabelWidget(QWidget * parent)
    {
        m_label = new QLabel(parent);
        return m_label;
    }

protected slots:
    void slotEditingFinished()
    {
        QVariant v;
        QString textValue = m_lineEdit->text();
        v = textValue;
        this->setValueToField(v);
    }

private:
    QPointer<QLineEdit> m_lineEdit;
    QPointer<QLabel>    m_label;
};

}

DemoPdmObj::DemoPdmObj()
{
    CAF_PDM_InitObject("DemoPdmObj", "", "", "");

    CAF_PDM_InitField(&f1, "f1",  1,  "Field 1", "", "","");
    CAF_PDM_InitField(&f2, "f2",  1,  "Field 2", "", "","");
    CAF_PDM_InitField(&f3, "f3",  1,  "Field 3", "", "","");
    CAF_PDM_InitField(&f4, "f4",  1,  "Field 4", "", "","");
    CAF_PDM_InitField(&f5, "f5",  1,  "Field 5", "", "","");

    f1.setEditorType(PdmUiFileEditor::editorName()); // "FileEditor" // typeid(PdmUiFileEditor) // "PdmUiFileEditor"
    this->addFieldToGroup(configname, 0, &f1);
    this->addFieldToGroup(configname, 0, &f2);

    this->addFieldToGroup(configname, 1, &f3);
    this->addFieldToGroup(configname, 1, &f4);


}

void DemoPdmObj::setUpUIConfiguration(QString uiConfigName, PdmUiConfiguration& uiConfig)
{
    if (uiConfigName == "DetailsView")
    {
        uiConfig->add(&f1);
        PdmUiGroup* group1 = uiConfig->addNewGroup("Name1");
        group1->add(&f2);
        PdmUiGroup* group2 = uiConfig->addNewGroup("Name2"); 
        group2->add(&f4);
        PdmUiGroup* group3 = group2->addNewGroup("Name3");
        group3->add(&f5);

        uiConfig->add(&f3);
        uiConfig->forgetRemainingFields();

       

    } else if (uiConfigName == "NormalView")
    {

    } else if (uiConfigName == "AnimationPanel")
    {

    }

}