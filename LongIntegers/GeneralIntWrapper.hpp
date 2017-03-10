template <class T>
GeneralIntWrapper<T>::GeneralIntWrapper()
{
	param1 = nullptr;
	param2 = nullptr;
	pResult = nullptr;
	bRunning = false;
	bFinished = false;
}

template <class T>
GeneralIntWrapper<T>::~GeneralIntWrapper()
{
	delete param1;
	delete param2;
}

template <class T>
QueueOfThreads<GeneralIntWrapper<T>> GeneralIntWrapper<T>::qot;

template <class T>
QueueOfThreads<GeneralIntWrapper<T>>* GeneralIntWrapper<T>::getQOT()
{
	return &qot;
}

template <class T>
int GeneralIntWrapper<T>::startProcess()
{
	bRunning = true;

	// Need to find a way to get this to call any method required
//	pResult = (T*)T::karatsubaMain(*param1, *param2, true);
	pResult = startMethod(*(param1), *(param2), true);


	bRunning = false;
	// Need a way this can inform QueueOfThreads that it has finished.
	bFinished = true;
	callbackObject->iHaveFinished(id);
	return 0;
}

template <class T>
void GeneralIntWrapper<T>::setCallback(ReceiveUpdateClass<GeneralIntWrapper<T>>* newcallbackObject)
{
	callbackObject = newcallbackObject;
}

template <class T>
void GeneralIntWrapper<T>::setStartMethod(Tfunction newMethod)
{
	startMethod = newMethod;
}

template <class T>
void GeneralIntWrapper<T>::setID(UINT newID)
{
	id = newID;
}

template <class T>
UINT GeneralIntWrapper<T>::getID()
{
	return id;
}

template <class T>
T* GeneralIntWrapper<T>::getResult()
{
	return pResult;
}

template <class T>
void GeneralIntWrapper<T>::setParams(const T& rParam1, const T& rParam2)
{
	// Copy the parameters. This requires that the assignment operator is overloaded to be a deep copy (should be)
	param1 = new T;
	param2 = new T;

	*param1 = rParam1;
	*param2 = rParam2;
}
