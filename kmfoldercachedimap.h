/**
 *  kmfoldercachedimap.cpp
 *
 *  Copyright (c) 2002-2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>
 *  Copyright (c) 2002-2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef kmfoldercachedimap_h
#define kmfoldercachedimap_h

#include <kdialogbase.h>
#include <kstandarddirs.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qdialog.h>

#include "kmfoldermaildir.h"
#include "kmfolderimap.h"
#include "kmacctimap.h"
#include "kmfoldertype.h"
#include "folderjob.h"
#include "cachedimapjob.h"

using KMail::FolderJob;
class KMAcctCachedImap;

namespace KMail {
   class AttachmentStrategy;
}
using KMail::AttachmentStrategy;

class DImapTroubleShootDialog : public KDialogBase
{
  Q_OBJECT
public:
  DImapTroubleShootDialog( QWidget* parent=0, const char* name=0 );

  static int run();

private slots:
  void slotRebuildIndex();
  void slotRebuildCache();

private:
  int rc;
};

class KMFolderCachedImap : public KMFolderMaildir
{
  Q_OBJECT

public:
  static QString cacheLocation() { 
     return locateLocal("data", "kmail/dimap" ); 
  } 
  /** Usually a parent is given. But in some cases there is no
    fitting parent object available. Then the name of the folder
    is used as the absolute path to the folder file. */
  KMFolderCachedImap(KMFolder* folder, const char* name=0);
  virtual ~KMFolderCachedImap();

  /** Returns the type of this folder */
  virtual KMFolderType folderType() const { return KMFolderTypeCachedImap; }

  /** Remove this folder */
  virtual void remove();

  /** Synchronize this folder and it's subfolders with the server */
  virtual void serverSync( bool suppressDialog );

  /** Force the sync state to be done. */
  void resetSyncState() { mSyncState = SYNC_STATE_INITIAL; }

  virtual void checkUidValidity();

  enum imapState { imapNoInformation=0, imapInProgress=1, imapFinished=2 };

  virtual imapState getContentState() { return mContentState; }
  virtual void setContentState(imapState state) { mContentState = state; }

  virtual imapState getSubfolderState() { return mSubfolderState; }
  virtual void setSubfolderState(imapState state) { mSubfolderState = state; }

  /** The path to the imap folder on the server */
  void setImapPath(const QString &path) { mImapPath = path; }
  QString imapPath() { return mImapPath; }

  /** The highest UID in the folder */
  void setLastUid( ulong uid );
  ulong lastUid();

  /** Find message by UID. Returns NULL if it doesn't exist */
  KMMsgBase* findByUID( ulong uid );

  /** The uidvalidity of the last update */
  void setUidValidity(const QString &validity) { mUidValidity = validity; }
  QString uidValidity() const { return mUidValidity; }

  /** The imap account associated with this folder */
  void setAccount(KMAcctCachedImap *acct);
  KMAcctCachedImap* account() const;

  /** Returns the filename of the uidcache file */
  QString uidCacheLocation() const;

  /** Read the uidValitidy and lastUid values from disk */
  int readUidCache();

  /** Write the uidValitidy and lastUid values to disk */
  int writeUidCache();

  /** Current progress status (in percents) */
  int progress() const { return mProgress; }

  /* Reimplemented from KMFolder. Moving is not supported, so aParent must be 0 */
  virtual int rename(const QString& aName, KMFolderDir *aParent=0);

  /* Reimplemented from KMFolderMaildir */
  virtual KMMessage* take(int idx);
  /* Reimplemented from KMFolderMaildir */
  virtual int addMsg(KMMessage* msg, int* index_return = 0);
  /* internal version that doesn't remove the X-UID header */
  virtual int addMsgInternal(KMMessage* msg, bool, int* index_return = 0);
  virtual int addMsgKeepUID(KMMessage* msg, int* index_return = 0) {
    return addMsgInternal(msg, false, index_return);
  }

  /* Reimplemented from KMFolderMaildir */
  virtual void removeMsg(int i, bool imapQuiet = FALSE);
  virtual void removeMsg(QPtrList<KMMessage> msgList, bool imapQuiet = FALSE)
    { FolderStorage::removeMsg(msgList, imapQuiet); }

  /**
   * Emit the folderComplete signal
   */
  void sendFolderComplete(bool success)
  { emit folderComplete(this, success); }

  /**
   * The silentUpload can be set to remove the folder upload error dialog
   */
  void setSilentUpload( bool silent ) { mSilentUpload = silent; }
  bool silentUpload() { return mSilentUpload; }

  virtual int createIndexFromContents()
    { return KMFolderMaildir::createIndexFromContents(); }

  // Mark for resync
  void resync() { mResync = true; }

  virtual void holdSyncs( bool hold ) { mHoldSyncs = hold; }

  void removeRightAway() { mRemoveRightAway = true; }

  /**
   * List a directory and add the contents to kmfoldermgr
   * returns false if the connection failed
   */
  bool listDirectory();

  /** Return the trash folder. */
  KMFolder* trashFolder() const;

protected slots:
  /**
   * Add the imap folders to the folder tree
   */
  void slotListEntries(KIO::Job * job, const KIO::UDSEntryList & uds);
  /**
   * Free the resources
   */
  void slotListResult(KIO::Job * job);

  void slotGetMessagesData(KIO::Job * job, const QByteArray & data);
  void getMessagesResult(KIO::Job * job, bool lastSet);
  void slotGetMessagesResult(KIO::Job * job);
  void slotGetLastMessagesResult(KIO::Job * job);
  void slotProgress(unsigned long done, unsigned long total);

  //virtual void slotCheckValidityResult(KIO::Job * job);
  virtual void listMessages();
  virtual void uploadNewMessages();
  virtual void uploadFlags();
  /* returns true if there were messages to delete
     on the server */
  virtual bool deleteMessages();
  virtual void createNewFolders();

  // Connected to the imap account
  void slotConnectionResult( int errorCode );

protected:
  void listDirectory2();


  /** Utility methods for syncing. Finds new messages
      in the local cache that must be uploaded */
  virtual QValueList<unsigned long> findNewMessages();
  /** Utility methods for syncing. Finds new subfolders
      in the local cache that must be created in the server */
  virtual QValueList<KMFolderCachedImap*> findNewFolders();

  /** This returns false if we have subfolders. Otherwise it returns ::canRemoveFolder() */
  virtual bool canRemoveFolder() const;

    /** Reimplemented from KMFolder */
  virtual FolderJob* doCreateJob( KMMessage *msg, FolderJob::JobType jt, KMFolder *folder,
                                  QString partSpecifier, const AttachmentStrategy *as ) const;
  virtual FolderJob* doCreateJob( QPtrList<KMMessage>& msgList, const QString& sets,
                                  FolderJob::JobType jt, KMFolder *folder ) const;

  virtual void timerEvent( QTimerEvent* );

public slots:
  /**
   * Add the data a KIO::Job retrieves to the buffer
   */
  void slotSimpleData(KIO::Job * job, const QByteArray & data);

  /**
   * Troubleshoot the IMAP cache
   */
  void slotTroubleshoot();

  /**
   * Sync this folder and it's subfolders.
   */
  void processNewMail();
  
private slots:
  void serverSyncInternal();

signals:
  void folderComplete(KMFolderCachedImap *folder, bool success);
  void listComplete( KMFolderCachedImap* );

  void listMessagesComplete();

  /* emitted at each state */
  void newState( const QString& folderName, int progressLevel, const QString& syncStatus );

  /** emitted when we enter the state "state" and
     have to process "number" items (for example messages
  */
  void syncState( int state, int number );

private:

  /** State variable for the synchronization mechanism */
  enum {
    SYNC_STATE_INITIAL,
    SYNC_STATE_PUT_MESSAGES,
    SYNC_STATE_UPLOAD_FLAGS,
    SYNC_STATE_CREATE_SUBFOLDERS,
    SYNC_STATE_LIST_SUBFOLDERS,
    SYNC_STATE_LIST_SUBFOLDERS2,
    SYNC_STATE_DELETE_SUBFOLDERS,
    SYNC_STATE_LIST_MESSAGES,
    SYNC_STATE_DELETE_MESSAGES,
    SYNC_STATE_EXPUNGE_MESSAGES,
    SYNC_STATE_GET_MESSAGES,
    SYNC_STATE_HANDLE_INBOX,
    SYNC_STATE_FIND_SUBFOLDERS,
    SYNC_STATE_SYNC_SUBFOLDERS,
    SYNC_STATE_CHECK_UIDVALIDITY
  } mSyncState;

  int mProgress;

  QString mUidValidity;
  QString     mImapPath;
  imapState   mContentState, mSubfolderState;
  QStringList mSubfolderNames, mSubfolderPaths, mSubfolderMimeTypes;

  bool        mHasInbox;
  bool        mIsSelected;
  bool        mCheckFlags;
  bool        mReadOnly;
  mutable QGuardedPtr<KMAcctCachedImap> mAccount;

  QValueList<ulong> uidsOnServer;
  QValueList<ulong> uidsForDeletionOnServer;
  QValueList<KMail::CachedImapJob::MsgForDownload> mMsgsForDownload;
  QValueList<ulong> mUidsForDownload;
  QStringList       foldersForDeletionOnServer;

  QValueList<KMFolderCachedImap*> mSubfoldersForSync;
  KMFolderCachedImap* mCurrentSubfolder;

  /* Mapping uid ->index
     Keep updated in addMsg, take and removeMsg */
  QMap<ulong,int> uidMap;
  bool uidMapDirty;
  ulong mLastUid;
  int uidWriteTimer;
  void reloadUidMap();

  QString state2String( int state ) const;
  bool mIsConnected;

  bool mSilentUpload;
  bool mFolderRemoved;
  bool mResync;
  bool mSuppressDialog;
  bool mHoldSyncs;
  bool mRemoveRightAway;
};

#endif /*kmfoldercachedimap_h*/
