"""autogenerated by genmsg_py from CableTension.msg. Do not edit."""
import roslib.message
import struct


class CableTension(roslib.message.Message):
  _md5sum = "b1e553f0fafb9cb708d68c56bd44d521"
  _type = "handle_msgs/CableTension"
  _has_header = False #flag to mark the presence of a Header object
  _full_text = """# The cable tension in one finger of the HANDLE hand.

float32 sensor1
float32 sensor2

"""
  __slots__ = ['sensor1','sensor2']
  _slot_types = ['float32','float32']

  def __init__(self, *args, **kwds):
    """
    Constructor. Any message fields that are implicitly/explicitly
    set to None will be assigned a default value. The recommend
    use is keyword arguments as this is more robust to future message
    changes.  You cannot mix in-order arguments and keyword arguments.
    
    The available fields are:
       sensor1,sensor2
    
    @param args: complete set of field values, in .msg order
    @param kwds: use keyword arguments corresponding to message field names
    to set specific fields. 
    """
    if args or kwds:
      super(CableTension, self).__init__(*args, **kwds)
      #message fields cannot be None, assign default values for those that are
      if self.sensor1 is None:
        self.sensor1 = 0.
      if self.sensor2 is None:
        self.sensor2 = 0.
    else:
      self.sensor1 = 0.
      self.sensor2 = 0.

  def _get_types(self):
    """
    internal API method
    """
    return self._slot_types

  def serialize(self, buff):
    """
    serialize message into buffer
    @param buff: buffer
    @type  buff: StringIO
    """
    try:
      _x = self
      buff.write(_struct_2f.pack(_x.sensor1, _x.sensor2))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize(self, str):
    """
    unpack serialized message in str into this message instance
    @param str: byte array of serialized message
    @type  str: str
    """
    try:
      end = 0
      _x = self
      start = end
      end += 8
      (_x.sensor1, _x.sensor2,) = _struct_2f.unpack(str[start:end])
      return self
    except struct.error as e:
      raise roslib.message.DeserializationError(e) #most likely buffer underfill


  def serialize_numpy(self, buff, numpy):
    """
    serialize message with numpy array types into buffer
    @param buff: buffer
    @type  buff: StringIO
    @param numpy: numpy python module
    @type  numpy module
    """
    try:
      _x = self
      buff.write(_struct_2f.pack(_x.sensor1, _x.sensor2))
    except struct.error as se: self._check_types(se)
    except TypeError as te: self._check_types(te)

  def deserialize_numpy(self, str, numpy):
    """
    unpack serialized message in str into this message instance using numpy for array types
    @param str: byte array of serialized message
    @type  str: str
    @param numpy: numpy python module
    @type  numpy: module
    """
    try:
      end = 0
      _x = self
      start = end
      end += 8
      (_x.sensor1, _x.sensor2,) = _struct_2f.unpack(str[start:end])
      return self
    except struct.error as e:
      raise roslib.message.DeserializationError(e) #most likely buffer underfill

_struct_I = roslib.message.struct_I
_struct_2f = struct.Struct("<2f")