object WebModule1: TWebModule1
  OnCreate = WebModuleCreate
  Actions = <
    item
      Default = True
      Name = 'DefaultHandler'
      PathInfo = '/'
      OnAction = WebModule1DefaultHandlerAction
    end
    item
      MethodType = mtGet
      Name = 'DeviceStatusAction'
      PathInfo = '/api/device/status'
      OnAction = WebModule1DeviceStatusActionAction
    end
    item
      MethodType = mtGet
      Name = 'DeviceIdentityAction'
      PathInfo = '/api/device/identity'
      OnAction = WebModule1DeviceIdentityActionAction
    end
    item
      MethodType = mtPost
      Name = 'MegaDisplayMessageAction'
      PathInfo = '/api/mega/display'
      OnAction = WebModule1MegaDisplayMessageActionAction
    end
    item
      MethodType = mtPost
      Name = 'MegaDisplayExpressionAction'
      PathInfo = '/api/mega/expression'
      OnAction = WebModule1MegaDisplayExpressionActionAction
    end
    item
      MethodType = mtPost
      Name = 'MegaDisplaySignatureAction'
      PathInfo = '/api/mega/signature'
      OnAction = WebModule1MegaDisplaySignatureActionAction
    end
    item
      MethodType = mtPost
      Name = 'WebModule1MegaSignatureStartActionAction'
      PathInfo = '/api/mega/signature/start'
      OnAction = WebModule1WebModule1MegaSignatureStartActionActionAction
    end
    item
      MethodType = mtGet
      Name = 'WebModule1MegaSignatureLiveActionAction'
      PathInfo = '/api/mega/signature/live'
      OnAction = WebModule1WebModule1MegaSignatureLiveActionActionAction
    end
    item
      MethodType = mtGet
      Name = 'WebModule1GallerySignaturesActionAction'
      PathInfo = '/api/gallery/signatures'
      OnAction = WebModule1WebModule1GallerySignaturesActionActionAction
    end
    item
      MethodType = mtGet
      Name = 'WebModule1GallerySignatureFileActionAction'
      PathInfo = '/api/gallery/signatures/file'
      OnAction = WebModule1WebModule1GallerySignatureFileActionActionAction
    end
    item
      MethodType = mtPost
      Name = 'GalleryPhotosAction'
      PathInfo = '/api/gallery/photos'
      OnAction = WebModule1GalleryPhotosActionAction
    end
    item
      MethodType = mtGet
      Name = 'GalleryPhotosListAction'
      PathInfo = '/api/gallery/photos'
      OnAction = WebModule1GalleryPhotosListActionAction
    end
    item
      MethodType = mtGet
      Name = 'GalleryPhotoFileAction'
      PathInfo = '/api/gallery/photos/file'
      OnAction = WebModule1GalleryPhotoFileActionAction
    end
    item
      MethodType = mtPost
      Name = 'DeviceCommandAction'
      PathInfo = '/api/device/command'
      OnAction = WebModule1DeviceCommandActionAction
    end>
  Height = 437
  Width = 623
  PixelsPerInch = 144
  object WebFileDispatcher1: TWebFileDispatcher
    WebFileExtensions = <
      item
        MimeType = 'text/html'
        Extensions = 'html;htm'
      end
      item
        MimeType = 'text/css'
        Extensions = 'css'
      end
      item
        MimeType = 'application/javascript'
        Extensions = 'js'
      end
      item
        MimeType = 'image/jpeg'
        Extensions = 'jpeg;jpg'
      end
      item
        MimeType = 'image/png'
        Extensions = 'png'
      end>
    WebDirectories = <
      item
        DirectoryAction = dirInclude
        DirectoryMask = '*'
      end
      item
        DirectoryAction = dirExclude
        DirectoryMask = '\templates\*'
      end>
    RootDirectory = 'wwwroot'
    VirtualPath = '/'
    Left = 424
    Top = 32
  end
  object WebStencilsEngine1: TWebStencilsEngine
    Dispatcher = WebFileDispatcher1
    PathTemplates = <>
    Left = 416
    Top = 152
  end
  object WebStencilsProcessor1: TWebStencilsProcessor
    Engine = WebStencilsEngine1
    Left = 416
    Top = 256
  end
end
