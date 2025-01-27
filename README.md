# DataPtrHookWin11
A .data pointer hook with communication for windows 11
![image](https://github.com/user-attachments/assets/da5d049a-110c-4e38-b1f6-930e36bdca26)

since windows 11, the data pointer is not in the .data section. the fuction pointer store in buffer pool, here code find the pool with pattern, is form 32GetSessionState(). then, we can get function pointer form the pool.
### origin function in IDA
```
__int64 (__fastcall *__fastcall NtUserSetGestureConfig(__int64 a1, unsigned int a2, unsigned int a3, __int64 a4, int a5))(__int64, _QWORD, _QWORD, __int64, int)
{
  __int64 (__fastcall *result)(__int64, _QWORD, _QWORD, __int64, int); // rax

  result = *(__int64 (__fastcall **)(__int64, _QWORD, _QWORD, __int64, int))(*(_QWORD *)(*(_QWORD *)(W32GetSessionState() + 136)
                                                                                       + 336i64)
                                                                           + 3120i64);
  if ( result )
    result = (__int64 (__fastcall *)(__int64, _QWORD, _QWORD, __int64, int))result(a1, a2, a3, a4, a5);
  return result;
}
```
