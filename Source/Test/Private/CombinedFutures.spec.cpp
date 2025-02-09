// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include <CoreMinimal.h>
#include <FutureExtensions.h>

#include "Helpers/TestHelpers.h"


#if WITH_DEV_AUTOMATION_TESTS

/************************************************************************/
/* FUTURE COMBINED SPEC                                                    */
/************************************************************************/

class FFutureTestSpec_Combined : public FFutureTestSpec
{
	GENERATE_SPEC(FFutureTestSpec_Basic, "FutureExtensions.Combined",
		EAutomationTestFlags::ProductFilter |
		EAutomationTestFlags::EditorContext |
		EAutomationTestFlags::ServerContext
	);

	FFutureTestSpec_Combined() : FFutureTestSpec()
	{
		DefaultTimeout = FTimespan::FromSeconds(0.2);
	}

	static constexpr int32 ErrorContext = 0xbaadf00d;
	static constexpr int32 ErrorCode = 0xdeadbeef;
};


void FFutureTestSpec_Combined::Define()
{
	Describe("WhenAll", [this]()
	{
		LatentIt("Success", [this](const auto& Done)
		{
				FE::WhenAll<int32>({ FE::MakeReadyFuture<int32>(1), FE::MakeReadyFuture<int32>(2), FE::MakeReadyFuture<int32>(4) })
					.Then([this](const TArray<int32>& Result)
						{
							int32 Total = 0;
							for (const int32 Value : Result)
							{
								Total += Value;
							}
							return Total;
						})
					.Then([this, Done](const FE::TExpected<int32>& Expected)
						{
							TestFalse("Result is an error", Expected.IsError());
							TestTrue("Result is completed", Expected.IsCompleted());
							TestEqual("Total", *Expected, 7);
							Done.Execute();
						});
			});

		LatentIt("Fail", [this](const auto& Done)
			{
				FE::WhenAll<int32>({ FE::MakeReadyFuture<int32>(1), FE::MakeReadyFuture<int32>(2), FE::MakeErrorFuture<int32>(FE::Error(ErrorCode, ErrorContext, TEXT("Bad times!"))) })
					.Then([this, Done](const FE::TExpected<TArray<int32>>& Expected)
						{
							TestTrue("Result is an error", Expected.IsError());
							TestFalse("Result is completed", Expected.IsCompleted());
							TestEqual("Captured String", *(Expected.GetError()->GetErrorInfo()), TEXT("Bad times!"));
							Done.Execute();
						});
			});
		});

	Describe("WhenAny", [this]()
		{
		LatentIt("Success", [this](const auto& Done)
			{
				FE::TExpectedPromise<int32> FirstPromise;
				FE::TExpectedPromise<int32> SecondPromise;
				FE::WhenAny<int32>({ FirstPromise.GetFuture(), SecondPromise.GetFuture() })
					.Then([this, Done](const FE::TExpected<int32>& Expected)
						{
							TestFalse("Result is an error", Expected.IsError());
							TestTrue("Result is completed", Expected.IsCompleted());
							TestEqual("Total", *Expected, 1);
							Done.Execute();
						});
				FirstPromise.SetValue(1);
				SecondPromise.SetValue(50);
			});

		LatentIt("Fail", [this](const auto& Done)
			{
				FE::TExpectedPromise<int32> FirstPromise;
				FE::TExpectedPromise<int32> SecondPromise;
				FE::WhenAny<int32>({ FirstPromise.GetFuture(), SecondPromise.GetFuture() })
					.Then([this, Done](const FE::TExpected<int32>& Expected)
						{
							TestTrue("Result is an error", Expected.IsError());
							TestFalse("Result is completed", Expected.IsCompleted());
							TestEqual("Captured String", *(Expected.GetError()->GetErrorInfo()), TEXT("Bad times!"));
							Done.Execute();
						});
				FirstPromise.SetValue(FE::Error(ErrorCode, ErrorContext, TEXT("Bad times!")));
				SecondPromise.SetValue(1);
			});
	});
}

#endif //WITH_DEV_AUTOMATION_TESTS
