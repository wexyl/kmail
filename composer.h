/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 1997 Markus Wuebben <markus.wuebben@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __KMAIL_COMPOSER_H__
#define __KMAIL_COMPOSER_H__

#include "secondarywindow.h"

#include <kurl.h>
#include <kglobalsettings.h>

#include <QString>

class KMMessage;
class KMFolder;
class KMMessagePart;
class MailComposerIface;

namespace KMail {

class Composer;

Composer *makeComposer( KMMessage *msg=0, uint identity=0 );

class Composer : public KMail::SecondaryWindow
{
  Q_OBJECT
  protected:
    Composer( const char *name=0 ) : KMail::SecondaryWindow( name ) {}

  public: // mailserviceimpl
    /**
     * From MailComposerIface
     */
    virtual void send( int how ) = 0;
    virtual void addAttachmentsAndSend( const KUrl::List &urls,
                                        const QString &comment, int how) = 0;
    virtual void addAttachment( const KUrl &url, const QString &comment ) = 0;
    virtual void addAttachment( const QString &name,
                                const QByteArray &cte,
                                const QByteArray &data,
                                const QByteArray &type,
                                const QByteArray &subType,
                                const QByteArray &paramAttr,
                                const QString &paramValue,
                                const QByteArray &contDisp ) = 0;
  public: // kmcommand
    virtual void setBody( const QString &body ) = 0;
    virtual QString dbusObjectPath() const = 0;
  public: // kmkernel, kmcommands, callback
    /**
     * Set the message the composer shall work with. This discards
     * previous messages without calling applyChanges() on them before.
     */
    virtual void setMsg( KMMessage *newMsg, bool mayAutoSign=true,
                         bool allowDecryption=false, bool isModified=false ) = 0;

    /**
     * Returns @c true while the message composing is in progress.
     */
    virtual bool isComposing() const = 0;

  public: // kmkernel
    /**
     * Set the filename which is used for autosaving.
     */
    virtual void setAutoSaveFilename( const QString &filename ) = 0;

  public: // kmkernel, callback
    /**
     * If this flag is set the message of the composer is deleted when
     * the composer is closed and the message was not sent. Default: false
     */
    virtual void setAutoDelete( bool f ) = 0;

  public: // kmcommand
    /**
     * If this folder is set, the original message is inserted back after
     * canceling
     */
    virtual void setFolder( KMFolder *aFolder ) = 0;

  public: // kmkernel, kmcommand, mailserviceimpl
    /**
     * Recode to the specified charset
     */
    virtual void setCharset( const QByteArray &aCharset, bool forceDefault=false ) = 0;

  public: // kmcommand
    /**
     * Sets the focus to the edit-widget and the cursor below the
     * "On ... you wrote" line when hasMessage is true.
     * Make sure you call this _after_ setMsg().
     */
    virtual void setReplyFocus( bool hasMessage=true ) = 0;

    /**
     * Sets the focus to the subject line edit. For use when creating a
     * message to a known recipient.
     */
    virtual void setFocusToSubject() = 0;

  public: // callback
    /** Disabled signing and encryption completely for this composer window. */
    virtual void setSigningAndEncryptionDisabled( bool v ) = 0;

  public slots: // kmkernel, callback
    virtual void slotSendNow() = 0;

  public slots: // kmkernel
    /**
       Tell the composer to always send the message, even if the user
       hasn't changed the next. This is useful if a message is
       autogenerated (e.g., via a D-Bus call), and the user should
       simply be able to confirm the message and send it.
    */
    virtual void slotSetAlwaysSend( bool bAlwaysSend ) = 0;

  public slots: // kmkernel, callback
    /**
     * Switch wordWrap on/off
     */
    virtual void slotWordWrapToggled( bool ) = 0;

  public slots: // kmkernel
    virtual void autoSaveMessage() = 0;

  public: // kmkernel, attachmentlistview
    virtual bool addAttach( const KUrl &url ) = 0;

    virtual void disableWordWrap() = 0;

    virtual void forceDisableHtml() = 0;

    virtual void disableRecipientNumberCheck() = 0;

    virtual void disableForgottenAttachmentsCheck() = 0;

    virtual void ignoreStickyFields() = 0;

  public: // kmcommand
    /**
     * Add an attachment to the list.
     */
    virtual void addAttach( KMMessagePart *msgPart ) = 0;
};

}

#endif // __KMAIL_COMPOSER_H__
