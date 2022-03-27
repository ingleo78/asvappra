#if defined (__GIO_GIO_H_INSIDE__) && !defined (GIO_COMPILATION)
//#error "Only <gio/gio.h> can be included directly."
#endif

#ifndef __GIO_TYPES_H__
#define __GIO_TYPES_H__

#include "../glib/glib-basic-types.h"
#include "../gobject/gobject.h"
#include "gioenums.h"

G_BEGIN_DECLS
typedef struct _GObject GObject;
typedef struct _GAppLaunchContext GAppLaunchContext;
typedef struct _GAppInfo GAppInfo;
typedef struct _GAsyncResult GAsyncResult;
typedef struct _GAsyncInitable GAsyncInitable;
typedef struct _GBufferedInputStream GBufferedInputStream;
typedef struct _GBufferedOutputStream GBufferedOutputStream;
typedef struct _GCancellable GCancellable;
typedef struct _GCharsetConverter GCharsetConverter;
typedef struct _GConverter GConverter;
typedef struct _GConverterInputStream GConverterInputStream;
typedef struct _GConverterOutputStream GConverterOutputStream;
typedef struct _GDataInputStream GDataInputStream;
typedef struct _GSimplePermission GSimplePermission;
typedef struct _GZlibCompressor GZlibCompressor;
typedef struct _GZlibDecompressor GZlibDecompressor;
typedef struct _GSimpleActionGroup GSimpleActionGroup;
typedef struct _GActionGroup GActionGroup;
typedef struct _GSimpleAction GSimpleAction;
typedef struct _GAction GAction;
typedef struct _GApplication GApplication;
typedef struct _GApplicationCommandLine GApplicationCommandLine;
typedef struct _GSettingsBackend GSettingsBackend;
typedef struct _GSettings GSettings;
typedef struct _GPermission GPermission;
typedef struct _GDrive GDrive;
typedef struct _GFileEnumerator GFileEnumerator;
typedef struct _GFileMonitor GFileMonitor;
typedef struct _GFilterInputStream GFilterInputStream;
typedef struct _GFilterOutputStream GFilterOutputStream;
typedef struct _GFile GFile;
typedef struct _GFileInfo GFileInfo;
typedef struct _GFileAttributeMatcher GFileAttributeMatcher;
typedef struct _GFileAttributeInfo GFileAttributeInfo;
typedef struct _GFileAttributeInfoList GFileAttributeInfoList;
typedef struct _GFileDescriptorBased GFileDescriptorBased;
typedef struct _GFileInputStream GFileInputStream;
typedef struct _GFileOutputStream GFileOutputStream;
typedef struct _GFileIOStream GFileIOStream;
typedef struct _GFileIcon GFileIcon;
typedef struct _GFilenameCompleter GFilenameCompleter;
typedef struct _GIcon GIcon; /* Dummy typedef */
typedef struct _GInetAddress GInetAddress;
typedef struct _GInetSocketAddress GInetSocketAddress;
typedef struct _GInputStream GInputStream;
typedef struct _GInitable GInitable;
typedef struct _GIOModule GIOModule;
typedef struct _GIOExtensionPoint GIOExtensionPoint;
typedef struct _GIOExtension GIOExtension;
typedef struct _GIOSchedulerJob GIOSchedulerJob;
typedef struct _GIOStreamAdapter GIOStreamAdapter;
typedef struct _GLoadableIcon GLoadableIcon;
typedef struct _GMemoryInputStream GMemoryInputStream;
typedef struct _GMemoryOutputStream GMemoryOutputStream;
typedef struct _GMount GMount;
typedef struct _GMountOperation GMountOperation;
typedef struct _GNetworkAddress GNetworkAddress;
typedef struct _GNetworkService GNetworkService;
typedef struct _GOutputStream GOutputStream;
typedef struct _GIOStream GIOStream;
typedef struct _GPollableInputStream GPollableInputStream;
typedef struct _GPollableOutputStream GPollableOutputStream;
typedef struct _GResolver GResolver;
typedef struct _GSeekable GSeekable;
typedef struct _GSimpleAsyncResult GSimpleAsyncResult;
typedef struct _GSocket GSocket;
typedef struct _GSocketControlMessage GSocketControlMessage;
typedef struct _GSocketClient GSocketClient;
typedef struct _GSocketConnection GSocketConnection;
typedef struct _GSocketListener GSocketListener;
typedef struct _GSocketService GSocketService;
typedef struct _GSocketAddress GSocketAddress;
typedef struct _GSocketAddressEnumerator GSocketAddressEnumerator;
typedef struct _GSocketConnectable GSocketConnectable;
typedef struct _GSrvTarget GSrvTarget;
typedef struct _GTcpConnection GTcpConnection;
typedef struct _GTcpWrapperConnection GTcpWrapperConnection;
typedef struct _GThreadedSocketService GThreadedSocketService;
typedef struct _GThemedIcon GThemedIcon;
typedef struct _GTlsCertificate GTlsCertificate;
typedef struct _GTlsClientConnection GTlsClientConnection;
typedef struct _GTlsClientContext GTlsClientContext;
typedef struct _GTlsConnection GTlsConnection;
typedef struct _GTlsContext GTlsContext;
typedef struct _GTlsServerConnection GTlsServerConnection;
typedef struct _GTlsServerContext GTlsServerContext;
typedef struct _GVfs GVfs;
typedef struct _GProxyResolver GProxyResolver;
typedef struct _GProxy GProxy;
typedef struct _GProxyAddress GProxyAddress;
typedef struct _GProxyAddressEnumerator	GProxyAddressEnumerator;
typedef struct _GVolume GVolume;
typedef struct _GVolumeMonitor GVolumeMonitor;
typedef void (*GAsyncReadyCallback)(GObject *source_object, GAsyncResult *res, gpointer user_data);
typedef void (*GFileProgressCallback)(goffset current_num_bytes, goffset total_num_bytes, gpointer user_data);
typedef gboolean (*GFileReadMoreCallback)(const char *file_contents, goffset file_size, gpointer callback_data);
typedef gboolean (*GIOSchedulerJobFunc)(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);
typedef void (*GSimpleAsyncThreadFunc)(GSimpleAsyncResult *res, GObject *object, GCancellable *cancellable);
typedef gboolean (*GSocketSourceFunc)(GSocket *socket, GIOCondition condition, gpointer user_data);
typedef struct _GInputVector GInputVector;
struct _GInputVector {
  gpointer buffer;
  gsize size;
};
typedef struct _GOutputVector GOutputVector;
struct _GOutputVector {
  gconstpointer buffer;
  gsize size;
};
struct _GAsyncResult {
  gpointer *ip;
};
typedef struct _GCredentials GCredentials;
typedef struct _GUnixCredentialsMessage GUnixCredentialsMessage;
typedef struct _GUnixFDList GUnixFDList;
typedef struct _GDBusMessage GDBusMessage;
typedef struct _GDBusConnection GDBusConnection;
typedef struct _GDBusProxy GDBusProxy;
typedef struct _GDBusMethodInvocation GDBusMethodInvocation;
typedef struct _GDBusServer GDBusServer;
typedef struct _GDBusAuthObserver GDBusAuthObserver;
typedef struct _GDBusErrorEntry GDBusErrorEntry;
typedef struct _GDBusInterfaceVTable GDBusInterfaceVTable;
typedef struct _GDBusSubtreeVTable GDBusSubtreeVTable;
typedef struct _GDBusAnnotationInfo GDBusAnnotationInfo;
typedef struct _GDBusArgInfo GDBusArgInfo;
typedef struct _GDBusMethodInfo GDBusMethodInfo;
typedef struct _GDBusSignalInfo GDBusSignalInfo;
typedef struct _GDBusPropertyInfo GDBusPropertyInfo;
typedef struct _GDBusInterfaceInfo GDBusInterfaceInfo;
typedef struct _GDBusNodeInfo GDBusNodeInfo;
typedef gboolean (*GCancellableSourceFunc)(GCancellable *cancellable, gpointer user_data);
typedef gboolean (*GPollableSourceFunc)(GObject *pollable_stream, gpointer user_data);
G_END_DECLS

#endif