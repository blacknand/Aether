# app/generated/__init__.py
# Make grpc-tools' absolute imports resolve when used as a package.
import importlib
import sys as _sys

def _alias(modname: str) -> None:
    pkgname = f"{__name__}.{modname}"           # e.g. "app.generated.order_management_pb2"
    mod = importlib.import_module(pkgname)      # import the real module
    _sys.modules.setdefault(modname, mod)       # register a top-level alias, e.g. "order_management_pb2"

for _n in ("order_management_pb2", "market_data_pb2"):
    try:
        _alias(_n)
    except Exception:
        pass
