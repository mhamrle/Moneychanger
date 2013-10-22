
#include <QDebug>
#include <QMessageBox>

#include "contactdetails.h"
#include "ui_contactdetails.h"

#include "Handlers/contacthandler.h"

#include "detailedit.h"

#include "UI/dlgnewcontact.h"



MTContactDetails::MTContactDetails(QWidget *parent) :
    MTEditDetails(parent),
    ui(new Ui::MTContactDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
}

MTContactDetails::~MTContactDetails()
{
    delete ui;
}

//virtual
void MTContactDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = MTContactHandler::getInstance()->DeleteContact(m_pOwner->m_qstrCurrentID.toInt());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->RefreshRecords();
        }
    }
}

//virtual
void MTContactDetails::AddButtonClicked()
{
    // -----------------------------------------------
    MTDlgNewContact theNewContact(this);
    // -----------------------------------------------
    theNewContact.setWindowTitle("Create New Contact");
    // -----------------------------------------------
    if (theNewContact.exec() == QDialog::Accepted)
    {
        QString nymID = theNewContact.GetId();

        qDebug() << QString("MTContactDetails::AddButtonClicked: OKAY was clicked. Value: %1").arg(nymID);

        //resume
        // TODO: Use the NymID we just obtained (theNewContact.GetId()) to create a new Contact.

        if (!nymID.isEmpty())
        {
            int nExisting = MTContactHandler::getInstance()->FindContactIDByNymID(nymID);

            if (nExisting > 0)
            {
                QString contactName = MTContactHandler::getInstance()->GetContactName(nExisting);

                QMessageBox::warning(this, QString("Contact Already Exists"),
                                     QString("Contact '%1' already exists with NymID: %2").arg(contactName).arg(nymID));
                return;
            }
            // -------------------------------------------------------
            //else (a contact doesn't already exist for that NymID)
            //
            int nContact  = MTContactHandler::getInstance()->CreateContactBasedOnNym(nymID);

            if (nContact <= 0)
            {
                QMessageBox::warning(this, QString("Failed creating contact"),
                                     QString("Failed trying to create contact for NymID: %1").arg(nymID));
                return;
            }
            // -------------------------------------------------------
            // else (Successfully created the new Contact...)
            // Now let's add this contact to the Map, and refresh the dialog,
            // and then set the new contact as the current one.
            //
            QString qstrContactID = QString("%1").arg(nContact);

            m_pOwner->m_map.insert(qstrContactID, QString("")); // Blank name. (To start.)
            m_pOwner->SetPreSelected(qstrContactID);
            m_pOwner->RefreshRecords();
        }
    } else {
      qDebug() << "MTContactDetails::AddButtonClicked: CANCEL was clicked";
    }
    // -----------------------------------------------
}

//virtual
void MTContactDetails::refresh(QString strID, QString strName)
{
    qDebug() << "MTContactDetails::refresh";

    ui->lineEditID  ->setText(strID);
    ui->lineEditName->setText(strName);

    int nContactID = strID.toInt();
    // --------------------------------------------
    QString strDetails;
    // --------------------------------------------
    {
        mapIDName theNymMap;

        if (MTContactHandler::getInstance()->GetNyms(theNymMap, nContactID))
        {
            strDetails += QString("Nyms:\n");

            for (mapIDName::iterator ii = theNymMap.begin(); ii != theNymMap.end(); ii++)
            {
                QString qstrNymID    = ii.key();
                QString qstrNymValue = ii.value();
                // -------------------------------------
                strDetails += QString("%1\n").arg(qstrNymID);
                // -------------------------------------
                mapIDName theServerMap;

                if (MTContactHandler::getInstance()->GetServers(theServerMap, qstrNymID))
                {
                    strDetails += QString("Found on servers:\n");

                    for (mapIDName::iterator iii = theServerMap.begin(); iii != theServerMap.end(); iii++)
                    {
                        QString qstrServerID    = iii.key();
                        QString qstrServerValue = iii.value();
                        // -------------------------------------
                        strDetails += QString("%1\n").arg(qstrServerID);
                        // -------------------------------------
                        mapIDName theAccountMap;

                        if (MTContactHandler::getInstance()->GetAccounts(theAccountMap, qstrNymID, qstrServerID, QString("")))
                        {
                            strDetails += QString("Where he owns the accounts:\n");

                            for (mapIDName::iterator iiii = theAccountMap.begin(); iiii != theAccountMap.end(); iiii++)
                            {
                                QString qstrAcctID    = iiii.key();
                                QString qstrAcctValue = iiii.value();
                                // -------------------------------------
                                strDetails += QString("%1\n").arg(qstrAcctID);
                                // -------------------------------------
                            } // for (accounts)
                        } // got accounts
                    } // for (servers)
                    strDetails += QString("\n");
                } // got servers
            } // for (nyms)
            strDetails += QString("\n");
        } // got nyms
    }
    // --------------------------------------------
    ui->plainTextEdit->setPlainText(strDetails);
}


// Add a "delete contact" function that the owner dialog can use.
// It will use this to delete the contact from the SQL db:
//
//         bool MTContactHandler::DeleteContact(int nContactID);

void MTContactDetails::on_lineEditName_editingFinished()
{
    int nContactID = m_pOwner->m_qstrCurrentID.toInt();

    if (nContactID > 0)
    {
        bool bSuccess = MTContactHandler::getInstance()->SetContactName(nContactID, ui->lineEditName->text());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

            m_pOwner->RefreshRecords();
        }
    }
}
